/*
   +----------------------------------------------------------------------+
   | Zend Engine, Removing unused variables                               |
   +----------------------------------------------------------------------+
   | Copyright © The PHP Group and Contributors.                          |
   +----------------------------------------------------------------------+
   | This source file is subject to the Modified BSD License that is      |
   | bundled with this package in the file LICENSE, and is available      |
   | through the World Wide Web at <https://www.php.net/license/>.        |
   |                                                                      |
   | SPDX-License-Identifier: BSD-3-Clause                                |
   +----------------------------------------------------------------------+
   | Authors: Nikita Popov <nikic@php.net>                                |
   +----------------------------------------------------------------------+
*/

#include "Optimizer/zend_optimizer_internal.h"
#include "zend_bitset.h"
#include "zend_observer.h"

/* Walk a scope-fn body's parent-CV references (body op1/op2/result IS_CV
 * operands plus ENTER_SCOPE_FUNC's literal map). If `vars_map`, rewrite each
 * reference with vars_map[old]; otherwise mark used_vars[old]. Recurses into
 * nested scope-fns since they share the same top-level parent's CVs. */
static void scope_fn_visit_parent_cvs(zend_op_array *scope_op, zend_bitset used_vars, const uint32_t *vars_map)
{
#define VISIT_REF(slot) do { \
	uint32_t old = VAR_NUM(slot); \
	if (vars_map) (slot) = NUM_VAR(vars_map[old]); \
	else zend_bitset_incl(used_vars, old); \
} while (0)
	bool in_body = false;
	for (uint32_t i = 0; i < scope_op->last; i++) {
		zend_op *opline = &scope_op->opcodes[i];
		if (opline->opcode == ZEND_ENTER_SCOPE_FUNC) {
			in_body = true;
			/* Mapping is pinned at literals[0..num_params). */
			uint32_t num_params = opline->op1.num;
			for (uint32_t j = 0; j < num_params; j++) {
				zval *zv = &scope_op->literals[j];
				uint32_t off = (uint32_t)Z_LVAL_P(zv);
				if (vars_map) ZVAL_LONG(zv, NUM_VAR(vars_map[VAR_NUM(off)]));
				else zend_bitset_incl(used_vars, VAR_NUM(off));
			}
			continue;
		}
		if (!in_body) continue;
		if (opline->op1_type == IS_CV) VISIT_REF(opline->op1.var);
		if (opline->op2_type == IS_CV) VISIT_REF(opline->op2.var);
		if (opline->result_type == IS_CV) VISIT_REF(opline->result.var);
	}
	for (uint32_t i = 0; i < scope_op->num_dynamic_func_defs; i++) {
		zend_op_array *nested = scope_op->dynamic_func_defs[i];
		if (nested->fn_flags2 & ZEND_ACC2_SCOPE_FUNC) {
			scope_fn_visit_parent_cvs(nested, used_vars, vars_map);
		}
	}
#undef VISIT_REF
}

/* This pass removes all CVs and temporaries that are completely unused. It does *not* merge any CVs or TMPs.
 * This pass does not operate on SSA form anymore. */
void zend_optimizer_compact_vars(zend_op_array *op_array) {
	int i;

	/* Scope-fn op_arrays don't own their CVs — those live on the top-level
	 * parent. We do all CV work for them as part of the parent's pass via
	 * mark/rename recursion below; calling here on a scope-fn directly
	 * would treat parent-CV references as out-of-range. */
	if (op_array->fn_flags2 & ZEND_ACC2_SCOPE_FUNC) {
		return;
	}

	ALLOCA_FLAG(use_heap1);
	ALLOCA_FLAG(use_heap2);
	uint32_t used_vars_len = zend_bitset_len(op_array->last_var + op_array->T);
	zend_bitset used_vars = ZEND_BITSET_ALLOCA(used_vars_len, use_heap1);
	uint32_t *vars_map = do_alloca((op_array->last_var + op_array->T) * sizeof(uint32_t), use_heap2);
	uint32_t num_cvs, num_tmps;

	/* Determine which CVs are used */
	zend_bitset_clear(used_vars, used_vars_len);
	for (i = 0; i < op_array->last; i++) {
		zend_op *opline = &op_array->opcodes[i];
		if (opline->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
			zend_bitset_incl(used_vars, VAR_NUM(opline->op1.var));
		}
		if (opline->op2_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
			zend_bitset_incl(used_vars, VAR_NUM(opline->op2.var));
		}
		if (opline->result_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
			zend_bitset_incl(used_vars, VAR_NUM(opline->result.var));
			if (opline->opcode == ZEND_ROPE_INIT) {
				uint32_t num = ((opline->extended_value * sizeof(zend_string*)) + (sizeof(zval) - 1)) / sizeof(zval);
				while (num > 1) {
					num--;
					zend_bitset_incl(used_vars, VAR_NUM(opline->result.var) + num);
				}
			}
		}
	}

	/* Scope-fn children's bodies reference our CVs at parent-relative indices
	 * baked into their op1.var/op2.var/result.var. Mark those uses too so we
	 * don't drop a CV that only the child reads. */
	for (i = 0; i < (int)op_array->num_dynamic_func_defs; i++) {
		zend_op_array *child = op_array->dynamic_func_defs[i];
		if (child->fn_flags2 & ZEND_ACC2_SCOPE_FUNC) {
			scope_fn_visit_parent_cvs(child, used_vars, NULL);
		}
	}

	num_cvs = 0;
	for (i = 0; i < op_array->last_var; i++) {
		if (zend_bitset_in(used_vars, i)) {
			vars_map[i] = num_cvs++;
		} else {
			vars_map[i] = (uint32_t) -1;
		}
	}

	num_tmps = 0;
	for (i = op_array->last_var; i < op_array->last_var + op_array->T; i++) {
		if (zend_bitset_in(used_vars, i)) {
			vars_map[i] = num_cvs + num_tmps++;
		} else {
			vars_map[i] = (uint32_t) -1;
		}
	}

	free_alloca(used_vars, use_heap1);
	if (num_cvs == op_array->last_var && num_tmps == op_array->T) {
		free_alloca(vars_map, use_heap2);
		return;
	}

	ZEND_ASSERT(num_cvs <= op_array->last_var);
	ZEND_ASSERT(num_tmps <= op_array->T);

	/* Update CV and TMP references in opcodes */
	for (i = 0; i < op_array->last; i++) {
		zend_op *opline = &op_array->opcodes[i];
		if (opline->op1_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
			opline->op1.var = NUM_VAR(vars_map[VAR_NUM(opline->op1.var)]);
		}
		if (opline->op2_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
			opline->op2.var = NUM_VAR(vars_map[VAR_NUM(opline->op2.var)]);
		}
		if (opline->result_type & (IS_CV|IS_VAR|IS_TMP_VAR)) {
			opline->result.var = NUM_VAR(vars_map[VAR_NUM(opline->result.var)]);
		}
	}

	/* Mirror the rename in scope-fn children's bodies so their parent-CV refs
	 * track the new indices. (Scope-fn TMP refs use a separate offset region
	 * and stay untouched here.) */
	for (i = 0; i < (int)op_array->num_dynamic_func_defs; i++) {
		zend_op_array *child = op_array->dynamic_func_defs[i];
		if (child->fn_flags2 & ZEND_ACC2_SCOPE_FUNC) {
			scope_fn_visit_parent_cvs(child, NULL, vars_map);
		}
	}

	/* Update CV name table */
	if (num_cvs != op_array->last_var) {
		if (num_cvs) {
			zend_string **names = safe_emalloc(sizeof(zend_string *), num_cvs, 0);
			for (i = 0; i < op_array->last_var; i++) {
				if (vars_map[i] != (uint32_t) -1) {
					names[vars_map[i]] = op_array->vars[i];
				} else {
					zend_string_release_ex(op_array->vars[i], 0);
				}
			}
			efree(op_array->vars);
			op_array->vars = names;
		} else {
			for (i = 0; i < op_array->last_var; i++) {
				zend_string_release_ex(op_array->vars[i], 0);
			}
			efree(op_array->vars);
			op_array->vars = NULL;
		}
		op_array->last_var = num_cvs;
	}

	op_array->T = num_tmps + ZEND_OBSERVER_ENABLED; // reserve last temporary for observers if enabled

	free_alloca(vars_map, use_heap2);
}
