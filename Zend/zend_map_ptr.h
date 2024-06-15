/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Zend Technologies Ltd. (http://www.zend.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Dmitry Stogov <dmitry@php.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef ZEND_MAP_PTR_H
#define ZEND_MAP_PTR_H

#include "zend_portability.h"
#ifndef _WIN32
#include <sys/mman.h>
#endif

typedef struct _zend_string zend_string;

#define ZEND_MAP_PTR_KIND_PURE    0
#define ZEND_MAP_PTR_KIND_INLINED 1

#if defined(MAP_FIXED) && SIZEOF_SIZE_T == 8
#define ZEND_MAP_PTR_KIND ZEND_MAP_PTR_KIND_INLINED
#else
#define ZEND_MAP_PTR_KIND ZEND_MAP_PTR_KIND_PURE
#endif

#define ZEND_MAP_PTR(ptr) \
	ptr ## __ptr
#define ZEND_MAP_PTR_DEF(type, name) \
	type ZEND_MAP_PTR(name)
#define ZEND_MAP_PTR_OFFSET2PTR(offset) \
	((void**)((char*)CG(map_ptr_base) + offset))
#define ZEND_MAP_PTR_PTR2OFFSET(ptr) \
	((void*)(((char*)(ptr)) - ((char*)CG(map_ptr_base))))
#define ZEND_MAP_PTR_INIT(ptr, val) do { \
		ZEND_MAP_PTR(ptr) = (val); \
	} while (0)
#define ZEND_MAP_PTR_NEW(ptr) do { \
		ZEND_MAP_PTR(ptr) = zend_map_ptr_new(); \
	} while (0)
#define ZEND_MAP_PTR_NEW_OFFSET() \
	((uint32_t)(uintptr_t)zend_map_ptr_new())
#define ZEND_MAP_PTR_IS_OFFSET(ptr) \
	(((uintptr_t)ZEND_MAP_PTR(ptr)) & 1L)
#define ZEND_MAP_PTR_GET(ptr) \
	((ZEND_MAP_PTR_IS_OFFSET(ptr) ? \
		ZEND_MAP_PTR_GET_IMM(ptr) : \
		((void*)(ZEND_MAP_PTR(ptr)))))
#define ZEND_MAP_PTR_GET_IMM(ptr) \
	(*ZEND_MAP_PTR_OFFSET2PTR((uintptr_t)ZEND_MAP_PTR(ptr)))
#define ZEND_MAP_PTR_SET(ptr, val) do { \
		if (ZEND_MAP_PTR_IS_OFFSET(ptr)) { \
			ZEND_MAP_PTR_SET_IMM(ptr, val); \
		} else { \
			ZEND_MAP_PTR_INIT(ptr, val); \
		} \
	} while (0)
#define ZEND_MAP_PTR_BIASED_BASE(real_base) \
	((void*)(((uintptr_t)(real_base)) - 1))
#define ZEND_MAP_PTR_SET_IMM(ptr, val) do { \
		void **__p = ZEND_MAP_PTR_OFFSET2PTR((uintptr_t)ZEND_MAP_PTR(ptr)); \
		*__p = (val); \
	} while (0)

#if ZEND_MAP_PTR_KIND == ZEND_MAP_PTR_KIND_PURE
# define ZEND_MAP_INLINED_PTR ZEND_MAP_PTR
# define ZEND_MAP_INLINED_PTR_DEF ZEND_MAP_PTR_DEF
# define ZEND_MAP_INLINED_PTR_INIT ZEND_MAP_PTR_INIT
# define ZEND_MAP_INLINED_PTR_INIT_NULL(ptr) ZEND_MAP_PTR_INIT(ptr, NULL)
# define ZEND_MAP_INLINED_PTR_NEW(ptr, size) ZEND_MAP_PTR_NEW(ptr)
# define ZEND_MAP_INLINED_PTR_GET ZEND_MAP_PTR_GET
# define ZEND_MAP_INLINED_PTR_SET ZEND_MAP_PTR_SET
# define ZEND_MAP_INLINED_PTR_IS_NULL(ptr) (ZEND_MAP_PTR_GET(ptr) == NULL)
#elif ZEND_MAP_PTR_KIND == ZEND_MAP_PTR_KIND_INLINED
# define ZEND_MAP_INLINED_PTR(ptr) \
	ptr ## __iptr
# define ZEND_MAP_INLINED_PTR_DEF(type, name) \
	type ZEND_MAP_INLINED_PTR(name)
# define ZEND_MAP_INLINED_PTR_INIT(ptr, val) do { \
		ZEND_ASSERT(val != NULL); \
		ZEND_MAP_INLINED_PTR(ptr) = ZEND_MAP_PTR_PTR2OFFSET(val); \
	} while (0)
# define ZEND_MAP_INLINED_PTR_INIT_NULL(ptr) do { \
		ZEND_MAP_INLINED_PTR(ptr) = NULL; \
	} while (0)
# define ZEND_MAP_INLINED_PTR_NEW(ptr, size) do { \
		ZEND_MAP_INLINED_PTR(ptr) = zend_map_inlined_ptr_new(size); \
	} while (0)
# define ZEND_MAP_INLINED_PTR_GET(ptr) \
	((void*)ZEND_MAP_PTR_OFFSET2PTR((uintptr_t)ZEND_MAP_INLINED_PTR(ptr)))
# define ZEND_MAP_INLINED_PTR_SET(ptr, val) do { \
		ZEND_MAP_INLINED_PTR(ptr) = (val); \
	} while (0)
# define ZEND_MAP_INLINED_PTR_IS_NULL(ptr) \
	(ZEND_MAP_INLINED_PTR(ptr) == NULL)
#else
# error "Unknown ZEND_MAP_PTR_KIND"
#endif

BEGIN_EXTERN_C()

typedef struct _zend_compiler_globals zend_compiler_globals;
ZEND_API void zend_map_ptr_init_base(zend_compiler_globals *compiler_globals);
ZEND_API void  zend_map_ptr_reset(void);
ZEND_API void *zend_map_inlined_ptr_new(size_t size);
static inline void *zend_map_ptr_new(void) {
    return zend_map_inlined_ptr_new(sizeof(void *));
}
ZEND_API void  zend_map_ptr_extend(size_t last);
ZEND_API void zend_alloc_ce_cache(zend_string *type_name);

END_EXTERN_C()

#endif /* ZEND_MAP_PTR_H */
