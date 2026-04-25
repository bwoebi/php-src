/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright © Zend Technologies Ltd., a subsidiary company of          |
   |     Perforce Software, Inc., and Contributors.                       |
   +----------------------------------------------------------------------+
   | This source file is subject to the Modified BSD License that is      |
   | bundled with this package in the file LICENSE, and is available      |
   | through the World Wide Web at <https://www.php.net/license/>.        |
   |                                                                      |
   | SPDX-License-Identifier: BSD-3-Clause                                |
   +----------------------------------------------------------------------+
   | Authors: Christian Seiler <chris_se@gmx.net>                         |
   |          Dmitry Stogov <dmitry@php.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef ZEND_CLOSURES_H
#define ZEND_CLOSURES_H

#include "zend_types.h"

BEGIN_EXTERN_C()

/* This macro depends on zend_closure structure layout */
#define ZEND_CLOSURE_OBJECT(op_array) \
	((zend_object*)((char*)(op_array) - sizeof(zend_object)))

/* Tag bits stored in scope_ed->extra_named_params (zval-aligned, low bits free).
 * Bit 0: this is a scope_ed (set by ZEND_ENTER_SCOPE_FUNC, checked by leave_helper).
 * Bit 1: this scope_ed attached an object (Fiber or Generator) to its closure
 *        on enter (must detach on leave). */
#define ZEND_SCOPE_ED_ENP_TAG_SCOPE_ED        (1u << 0)
#define ZEND_SCOPE_ED_ENP_TAG_OBJECT_ATTACHED (1u << 1)
#define ZEND_SCOPE_ED_ENP_TAG_MASK \
	(ZEND_SCOPE_ED_ENP_TAG_SCOPE_ED | ZEND_SCOPE_ED_ENP_TAG_OBJECT_ATTACHED)

void zend_register_closure_ce(void);
void zend_closure_bind_var(zval *closure_zv, zend_string *var_name, zval *var);
void zend_closure_bind_var_ex(zval *closure_zv, uint32_t offset, zval *val);
void zend_closure_from_frame(zval *closure_zv, const zend_execute_data *frame);

extern ZEND_API zend_class_entry *zend_ce_closure;

ZEND_API void zend_create_closure(zval *res, zend_function *op_array, zend_class_entry *scope, zend_class_entry *called_scope, zval *this_ptr);
ZEND_API void zend_create_fake_closure(zval *res, zend_function *op_array, zend_class_entry *scope, zend_class_entry *called_scope, zval *this_ptr);
ZEND_API zend_function *zend_get_closure_invoke_method(zend_object *obj);
ZEND_API const zend_function *zend_get_closure_method_def(zend_object *obj);
ZEND_API zval* zend_get_closure_this_ptr(zval *obj);
ZEND_API zval* zend_closure_get_this_ptr_ptr(zend_object *obj);

/* For scope-fn closures: returns &closure->attached_object so callers can
 * load/store the back-reference. The attached object is either a Fiber
 * (parent-exit drives a forced unwind) or a Generator (parent-exit force-
 * destructs it). Disambiguate by ->ce. Caller must ensure the closure is
 * a scope function (ZEND_ACC2_SCOPE_FUNC). */
ZEND_API zend_object **zend_closure_get_attached_object_ptr(zend_object *obj);

END_EXTERN_C()

#endif
