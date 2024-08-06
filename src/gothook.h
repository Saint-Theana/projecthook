/*
 * libdetour.h - Simple C/C++ library for detour hooking in Linux and Windows.
 * See: https://github.com/8dcc/libdetour
 * Copyright (C) 2024 8dcc
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBDETOUR_H_
#define LIBDETOUR_H_ 1

#include <stdint.h>
#include <stdbool.h>

#ifdef __i386__
#define LIBDETOUR_JMP_SZ_ 7 /* Size of jmp instructions in 32-bit */
#else
#define LIBDETOUR_JMP_SZ_ 12 /* Size of jmp instructions in 64-bit */
#endif

typedef struct {
    bool detoured;
    void* orig;
    void* hook;
    uint8_t jmp_bytes[LIBDETOUR_JMP_SZ_];
    uint8_t saved_bytes[LIBDETOUR_JMP_SZ_];
} libdetour_ctx_t;

/*----------------------------------------------------------------------------*/

void libdetour_init(libdetour_ctx_t* ctx, void* orig, void* hook);
bool libdetour_add(libdetour_ctx_t* ctx);
bool libdetour_del(libdetour_ctx_t* ctx);

/*----------------------------------------------------------------------------*/

/* Declare the prototype of the original function */
#define LIBDETOUR_DECL_TYPE(FUNCRET, FUNCNAME, ...) \
    typedef FUNCRET (*libdetour_##FUNCNAME##_t)(__VA_ARGS__)

/* Remove detour hook, call original, detour again.
 * Keep in mind that:
 *   - The returned value of the original function is not stored. If the
 *     function is not void, and you care about the return value, use
 *     LIBDETOUR_ORIG_GET() instead.
 *   - FUNCNAME should be the same name passed to LIBDETOUR_DECL_TYPE, without
 *     the prefix or subfix added by the macro ("libdetour_X_t") */
#define LIBDETOUR_ORIG_CALL(CTX_PTR, FUNCNAME, ...)                 \
    do {                                                            \
        libdetour_del(CTX_PTR);                                     \
        ((libdetour_##FUNCNAME##_t)((CTX_PTR)->orig))(__VA_ARGS__); \
        libdetour_add(CTX_PTR);                                     \
    } while (0)

/* Same as LIBDETOUR_ORIG_CALL, but accepts an extra parameter for storing the
 * returned value of the original function */
#define LIBDETOUR_ORIG_GET(CTX_PTR, OUT_VAR, FUNCNAME, ...)                   \
    do {                                                                      \
        libdetour_del(CTX_PTR);                                               \
        OUT_VAR = ((libdetour_##FUNCNAME##_t)((CTX_PTR)->orig))(__VA_ARGS__); \
        libdetour_add(CTX_PTR);                                               \
    } while (0)

#endif /* LIBDETOUR_H_ */