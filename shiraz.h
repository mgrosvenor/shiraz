/*
 * Shiraz (SRZ)
 * ================
 * This is an easy to use options parsing library.
 *
 * Documentation
 * =============
 * Please see README.md
 *
 * Legal Stuff
 * ============
 * Copyright (c) 2021, Matthew P. Grosvenor
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SRZH_
#define SRZH_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include <ctype.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include <getopt.h>


/*
 * Build Time Parameters
 * ===========================================================================
 */


#ifndef SRZ_HARD_EXIT
#define SRZ_HARD_EXIT  1 //Fail to exit(0xDEAD).
#endif

#ifndef SRZ_PEDANTIC
#define SRZ_PEDANTIC 1 //If this is set, pedantic error checking is performed
#endif

#ifndef SRZ_DEBUG
#define SRZ_DEBUG 0 //If this is set, debug printing is enabled
#endif

#define SRZ_SOPTS_MAX 256 //Maximum number of charters to use for short options
#define SRZ_LOPTS_MAX 128 //Maximum number of long options
#define SRZ_OPTS_MAX  SRZ_LOPTS_MAX //Maximum number of options in total. Should not be less than SRZ_LOPTS_MAX


/*
 * Forward declarations of the the SRZ interface functions
 * ===========================================================================
 */


#if SRZ_PEDANTIC
#define ifp(p, e) if(p){e}
#else
#define ifp(p,e)
#endif

typedef enum {
    SRZ_VAL_BOOL,
    SRZ_VAL_INT,
    SRZ_VAL_INT8,
    SRZ_VAL_INT16,
    SRZ_VAL_INT32,
    SRZ_VAL_INT64,
    SRZ_VAL_UINT,
    SRZ_VAL_UINT8,
    SRZ_VAL_UINT16,
    SRZ_VAL_UINT32,
    SRZ_VAL_UINT64,
    SRZ_VAL_FLOAT,
    SRZ_VAL_DOUBLE,
    SRZ_VAL_STR,
    SRZ_VAL_ENUM,
} srz_val_type_t;

typedef enum {
    SRZ_OPT_NONE,
    SRZ_OPT_SHORT,
    SRZ_OPT_LONG,
    SRZ_OPT_POS,
    SRZ_OPT_UNKOWN_NONE,
    SRZ_OPT_UNKOWN_LONG,
    SRZ_OPT_UNKOWN_SHORT,
    SRZ_OPT_ARG_MISSING_NONE,
    SRZ_OPT_ARG_MISSING_LONG,
    SRZ_OPT_ARG_MISSING_SHORT,
} srz_opt_type_t;

typedef struct srz_enum {
    int val;
    char* str;
} srz_enum_t;

typedef struct srz_val {
    srz_val_type_t type;
    bool is_vector;
    union {
        bool b;
        int64_t i;
        uint64_t u;
        double f;
        char* s;
    } init;

    void* dest;

    srz_enum_t* enm_map;
} srz_val_t;

typedef enum {
    SRZ_ARG_NON, //Argument is not expected (flag)
    SRZ_ARG_OPT, //Argument is optional
    SRZ_ARG_REQ, //Argument is required
    SRZ_ARG_POS, //Argument is positional
} srz_atype_t;

typedef struct srz_option {
    int ident;
    srz_atype_t atype;
    char* srt;
    char* lng;
    char* desc;
    srz_val_t val;
    bool fin; //Indicates end of argument list
} srz_opt_t;

#define SRZ_FIN { .fin = true }
#define SRZ_REQ(IDENT, SHORT, LONG, DESCR) \
{                                          \
    .ident    = IDENT,                     \
    .atype    = SRZ_ARG_REQ,               \
    .srt      = SHORT,                     \
    .lng      = LONG,                      \
    .desc     = DESCR,                     \
    .fin      = false,                     \
    .val      = {0},                       \
}
#define SRZ_FLG(IDENT, SHORT, LONG, DESCR) \
{                                          \
    .ident    = IDENT,                     \
    .atype    = SRZ_ARG_NON,               \
    .srt      = SHORT,                     \
    .lng      = LONG,                      \
    .desc     = DESCR,                     \
    .fin      = false,                     \
    .val      = {0},                       \
}
#define SRZ_POS(IDENT, SHORT, LONG, DESCR) \
{                                          \
    .ident    = IDENT,                     \
    .atype    = SRZ_ARG_POS,               \
    .srt      = SHORT,                     \
    .lng      = LONG,                      \
    .desc     = DESCR,                     \
    .fin      = false,                     \
    .val      = {0},                       \
}


/*
 * ******** !!!! WARNING !!!! ********
 * Long options (and by GNU extension short options as well) **can** support "optional" arguments.
 * Seriously!! Don't use this feature! There's lots of reasons:
 * - It is likely to cause confusion to users when it's done wrong.
 * - Only long options are supported widely (GNU extension for short optionals)
 * - Only "=" assignment is supported (not spaces) for long opts, only no space is supported for short opts
 * */
#define SRZ_OPT(IDENT, SHORT, LONG, DESCR) \
{                                          \
    .ident    = IDENT,                     \
    .atype    = SRZ_ARG_OPT,               \
    .srt      = SHORT,                     \
    .lng      = LONG,                      \
    .desc     = DESCR,                     \
    .fin      = false,                     \
    .val      = {0},                       \
}


//Remember to update the string translation table srz_error_en[]
typedef enum {
    SRZ_ERR_NONE = 0,
    SRZ_ERR_SOPTS_MAX_TOO_SMALL,
    SRZ_ERR_SOPT_DUP,
    SRZ_ERR_SOPT_OPTIONAL,
    SRZ_ERR_SOPT_TOO_LONG,
    SRZ_ERR_LOPT_DUP,
    SRZ_ERR_LOPTS_MAX_TOO_SMALL,
    SRZ_ERR_UNKNOWN_ARG_TYPE,
    SRZ_ERR_INTERNAL,
    SRZ_ERR_INTERNAL2,
    SRZ_ERR_MULTI_POSITIONAL,
    SRZ_ERR_POSTIONAL_FOUND,
    SRZ_ERR_NO_SHORT_LONG,
    SRZ_ERR_OPTS_MAX_TOO_SMALL,
    SRZ_ERR_NO_OPTS_ADDED,
    SRZ_ERR_LAST, //Last error code, use this as a base for custom errors
} srz_errno_t;

typedef struct srz_error_en {
    srz_errno_t err_no;
    const char* err_str;
} srz_err_t;


typedef int (* srz_opt_handler_t)(srz_opt_type_t opt_type, const srz_opt_t* const opt, const char* const optval, void* user);
srz_errno_t srz_parse_ex(int argc, char** argv, srz_opt_t* opts, srz_opt_handler_t opt_handler, void* user);

int srz_parse(int argc, char** argv);

#define _srz_add_x(n,T) \
    int srz_add_##n(char* sopt, char* lopt, char* desc, T* dest, T init)

#define _srz_add_X(N,T) \
    int srz_add_##N(char* sopt, char* lopt, char* desc, T** dest)

#define _srz_add_xX(n,N,T) \
    _srz_add_x(n,T); \
    _srz_add_X(N,T)

_srz_add_xX(i, I, int);
_srz_add_xX(i8,  I8,  int8_t);
_srz_add_xX(i16, I16, int16_t);
_srz_add_xX(i32, I32, int32_t);
_srz_add_xX(i64, I64, int64_t);

_srz_add_xX(u, U, unsigned);
_srz_add_xX(u8, U8, uint8_t);
_srz_add_xX(u16, U16, uint16_t);
_srz_add_xX(u32, U32, uint32_t);
_srz_add_xX(u64, U64, uint64_t);

_srz_add_xX(f, F, float);
_srz_add_xX(d, D, double);

_srz_add_xX(s, S, char*);

#if __STDC__==1 && __STDC_VERSION__ >= 201112L
#define srz_opt(sopt, lopt, descr, dest, init) _Generic( (init), \
              int8_t: srz_add_i8,                                \
              int16_t: srz_add_i16,                              \
              int32_t: srz_add_i32,                              \
              int64_t: srz_add_i64,                              \
              uint8_t: srz_add_u8,                               \
              uint16_t: srz_add_u16,                             \
              uint32_t: srz_add_u32,                             \
              uint64_t: srz_add_u64,                             \
              float: srz_add_f,                                  \
              double: srz_add_d,                                 \
              char*: srz_add_s                                   \
              )( sopt, lopt, descr, dest, init)

#define srz_vec(sopt, lopt, descr, dest) _Generic( (dest),   \
              int8_t**: srz_add_I8,                          \
              int16_t**: srz_add_I16,                        \
              int32_t**: srz_add_I32,                        \
              int64_t**: srz_add_I64,                        \
              uint8_t**: srz_add_U8,                         \
              uint16_t**: srz_add_U16,                       \
              uint32_t**: srz_add_U32,                       \
              uint64_t**: srz_add_U64,                       \
              float**: srz_add_F,                            \
              double**: srz_add_D,                           \
              char***: srz_add_S                             \
              )(sopt, lopt, descr, dest)
#endif

#define srz_flg(sopt, lopt, desc, dest) \
    srz_add_i(sopt, lopt, desc, dest, 0)

int srz_add_e(char* sopt, char* lopt, char* desc, int* dest, int init, srz_enum_t* map);
#define srz_enm(sopt, lopt, desc, dest, init, map) \
    srz_add_e(sopt, lopt, desc, dest, init, map)

int srz_add_E(char* sopt, char* lopt, char* desc, int** dest, srz_enum_t* map);
#define srz_ens(sopt, lopt, desc, dest, map) \
    srz_add_E(sopt, lopt, desc, dest, map)

int srz_add_P(char* sopt, char* lopt, char* desc, char*** dest);
#define srz_pos(sopt, lopt, desc, dest) \
    srz_add_P(sopt, lopt, desc, dest)



typedef struct srz {
    srz_opt_t opts[SRZ_OPTS_MAX];
    srz_errno_t errno;
    bool init_complete;
    size_t opt_idx;
    bool help;
} srz_t;

extern  srz_t ___srz___;

/*
* Implementation!
* ============================================================================
*/
#ifndef SRZ_HONLY


static struct srz_error_en srz_error_en[] = {
    {SRZ_ERR_NONE,                "No error"},
    {SRZ_ERR_SOPTS_MAX_TOO_SMALL, "The short options string space is too small, try enlarging SRZ_SOPTS_MAX and recompiling" },
    {SRZ_ERR_SOPT_DUP,            "Duplicate short options entry. Remove or rename this short option name" },
    {SRZ_ERR_SOPT_OPTIONAL,       "A short option cannot have an optional argument. Remove short option or use ARG_NON or ARG_REQ argument type"},
    {SRZ_ERR_SOPT_TOO_LONG,       "Short options should be a single character long only. Remove excess characters, or use a long option instead" },
    {SRZ_ERR_LOPT_DUP,            "Duplicate long options entry. Remove or rename this short option name" },
    {SRZ_ERR_LOPTS_MAX_TOO_SMALL, "The long options memory space is too small, try enlarging SRZ_LOPTS_MAX and recompiling" },
    {SRZ_ERR_UNKNOWN_ARG_TYPE,    "The option argument type is unkown. Valid values are ARG_NON, ARG_OPT, ARG_REQ"},
    {SRZ_ERR_INTERNAL,            "Internal consistency error. Option found in long_opts structure, but no in SRZ options"},
    {SRZ_ERR_INTERNAL2,            "Internal consistency error. Unknown return type from _srz_fuzzy_find() function."},
    {SRZ_ERR_MULTI_POSITIONAL,    "Multiple positional options found (ARG_POS), only 1 permitted. Remove additional options"},
    {SRZ_ERR_POSTIONAL_FOUND,     "Positional arguments found, but there is no positional option. Add ARG_POS, or remove positional arguments"},
    {SRZ_ERR_NO_SHORT_LONG,       "Neither a short or long option string were supplied. Both cannot be blank. Either a short or long option string is required"},
    {SRZ_ERR_OPTS_MAX_TOO_SMALL,  "The options memory space is too small, reduce the number of options in use or enlarge SRZ_OPTS_MAX and recompile" },
    {SRZ_ERR_NO_OPTS_ADDED,       "No Shiraz options have been added. Use szr_opt(), srz_vec(), srz_flg() and related functions to add options"},
    {0,                           0 }
};

const char* srz_err2str_en(srz_errno_t err_no)
{
    for(srz_err_t* e = srz_error_en; e->err_str; e++ ){
        if(e->err_no == err_no){
            return e->err_str;
        }
    }

    return "Error number not found";
}

/*
 * This debugging code liberally borrowed and adapted from libchaste by
 * M.Grosvenor BSD 3 clause license. https://github.com/mgrosvenor/libchaste
 */
typedef enum {
    SRZ_MSG_DBG,
    SRZ_MSG_WARN,
    SRZ_MSG_ERR,
} srz_dbg_e;

#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif


#ifdef SRZ_DEBUG
    #define SRZ_FAIL( /*format, args*/...)  srz_err_helper(__VA_ARGS__, "")
    #define srz_err_helper(format, ...) _srz_msg(SRZ_MSG_ERR, __LINE__, __FILE__, __FUNCTION__, format, __VA_ARGS__ )
    #define SRZ_WARN( /*format, args*/...)  srz_warn_helper(__VA_ARGS__, "")
    #define srz_warn_helper(format, ...) _srz_msg(SRZ_MSG_WARN,__LINE__, __FILE__, __FUNCTION__, format, __VA_ARGS__ )
    #define SRZ_DBG( /*format, args*/...)  srz_debug_helper(__VA_ARGS__, "")
    #define srz_debug_helper(format, ...) _srz_msg(SRZ_MSG_DBG,__LINE__, __FILE__, __FUNCTION__, format, __VA_ARGS__ )
#else
    #define SRZ_FAIL( /*format, args*/...)
    #define SRZ_WARN( /*format, args*/...)
    #define SRZ_DBG( /*format, args*/...)
#endif

//__attribute__ ((format (printf, 5, 6)))
static inline void _srz_msg(srz_dbg_e mode, int ln, const char* fn, const char* fu, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    const char* mode_str = NULL;
    switch(mode){
        case SRZ_MSG_ERR:  mode_str = "Error  :"; break;
        case SRZ_MSG_DBG:  mode_str = "Debug  :"; break;
        case SRZ_MSG_WARN: mode_str = "Warning:"; break;
        default:
            return;
    }
    fprintf(stderr, "[%s %s:%i:%s()]  ", mode_str, basename((char*)fn), ln, fu);
    vfprintf(stderr, msg, args);

    if(mode == SRZ_MSG_ERR && SRZ_HARD_EXIT){
        exit(0xDEAD);
    }
    va_end(args);
}


static inline int isempty(const char* s)
{
    if(s == NULL){
        return 1;
    }
    if(strlen(s) == 0){
        return 1;
    }

    return 0;
}


static inline srz_opt_t* _srz_find_short(srz_opt_t opts[], char s)
{
    for(srz_opt_t* opt = opts; !opt->fin; opt++){
        const char* srt = opt->srt;
        if(isempty(srt)){
            continue;
        }

        if(strlen(srt) > 1){
            continue; //This is really a long option in disguise
        }

        if(srt[0] == s){
            return opt;
        }
    }

    return NULL;
}

static inline srz_opt_t* _srz_find_long(srz_opt_t opts[], const char* l, int* ignore)
{
    for(srz_opt_t* opt = opts; !opt->fin; opt++){
        if(ignore && opt->ident == *ignore){
            continue;
        }
        const char* lng = opt->lng;
        if(isempty(lng)){
            continue;
        }

        if(strcmp(l,lng) == 0){
            return opt;
        }
    }

    return NULL;
}


/*
 * The following code greatfully borrowed thanks to Titus Wormer
 *
 * (The MIT License)
 *
 * Copyright (c) 2015 Titus Wormer <tituswormer@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 */

static inline size_t _srz_levenshtein_n(const char* a, const size_t length, const char* b, const size_t bLength)
{
    // Shortcut optimizations / degenerate cases.
    if(a == b){
        return 0;
    }

    if(length == 0){
        return bLength;
    }

    if(bLength == 0){
        return length;
    }

    size_t* cache = calloc(length, sizeof(size_t));
    size_t index = 0;
    size_t bIndex = 0;
    size_t distance;
    size_t bDistance;
    size_t result;
    char code;

    // initialize the vector.
    while(index < length){
        cache[index] = index + 1;
        index++;
    }

    // Loop.
    while(bIndex < bLength){
        code = b[bIndex];
        result = distance = bIndex++;
        index = SIZE_MAX;

        while(++index < length){
            bDistance = code == a[index] ? distance : distance + 1;
            distance = cache[index];

            cache[index] = result = distance > result
                                    ? bDistance > result
                                      ? result + 1
                                      : bDistance
                                    : bDistance > distance
                                      ? distance + 1
                                      : bDistance;
        }
    }

    free(cache);

    return result;
}

static inline size_t _srz_levenshtein(const char* a, const char* b)
{
    const size_t length = strlen(a);
    const size_t bLength = strlen(b);

    return _srz_levenshtein_n(a, length, b, bLength);
}


//Fuzzy search to try and find the best match for an option
static inline srz_opt_t* _srz_fuzzy_find_opt(srz_opt_t opts[], const char* s, srz_opt_type_t* opt_type_o)
{
    //Trivial escape
    if(isempty(s)){
        //We're never going to match anything
        *opt_type_o = SRZ_OPT_NONE;
        return NULL;
    }

    //Try an exact match for the short string
    srz_opt_t* result = NULL;
    if(strlen(s) == 1){
        result = _srz_find_short(opts,s[0]);
        if(result){
            *opt_type_o = SRZ_OPT_SHORT;
            return result;
        }
    }

    //Try an exact match for the long string
    result = _srz_find_long(opts,s,NULL);
    if(result){
        *opt_type_o = SRZ_OPT_LONG;
        return result;
    }

    //Maybe it's a short option with a dash?
    if(strlen(s) == 2 && s[1] == '-'){
        result = _srz_find_short(opts,s[1]);
        if(result){
            *opt_type_o = SRZ_OPT_SHORT;
            return result;
        }
    }

    //Maybe it's a long option with one dash?
    if(strlen(s) > 2 && s[1] == '-' ){
        result = _srz_find_long(opts, s+1, NULL);
        if(result){
            *opt_type_o = SRZ_OPT_LONG;
            return result;
        }
    }

    //Maybe it's a long option with two dashes?
    if(strlen(s) > 3 && s[1] == '-' && s[2] == '-'){
        result = _srz_find_long(opts, s+2, NULL);
        if(result){
            *opt_type_o = SRZ_OPT_LONG;
            return result;
        }
    }

    //We've tried hard to find an exact match, now try fuzzy matching

    size_t best_match_lev = ~0;
    srz_opt_t* best_match = NULL;
    *opt_type_o = SRZ_OPT_NONE;

    for(srz_opt_t* opt = opts; !opt->fin; opt++){
        const char* lng = opt->lng;
        const char* srt = opt->srt;

        if(!isempty(lng)){
            const size_t match = _srz_levenshtein(lng,s);
            if(match < best_match_lev){
                best_match_lev = match;
                best_match = opt;
                *opt_type_o = SRZ_OPT_LONG;
            }
        }

        if(!isempty(srt)){
            const size_t match = _srz_levenshtein(srt,s);
            if(match < best_match_lev){
                best_match_lev = match;
                best_match = opt;
                *opt_type_o = SRZ_OPT_SHORT;
            }
        }
    }

    return best_match;
}

static inline int _srz_positional_count(srz_opt_t opts[])
{
    int result = 0;
    for(srz_opt_t* opt = opts; !opt->fin; opt++){

        if(opt->atype == SRZ_ARG_POS){
            result++;
        }
    }

    return result;
}

static inline srz_opt_t* _srz_get_positional(srz_opt_t opts[])
{
    for(srz_opt_t* opt = opts; !opt->fin; opt++){

        if(opt->atype == SRZ_ARG_POS){
            return opt;
        }
    }

    return NULL;
}

static inline int _srz_no_short_long(srz_opt_t opts[])
{
    for(srz_opt_t* opt = opts; !opt->fin; opt++){
        if(opt->atype != SRZ_ARG_POS){
            if(isempty(opt->srt) && isempty(opt->lng)){
                return 1;
            }
        }
    }

    return 0;
}


static inline srz_errno_t _srz_build_short_opts(srz_opt_t opts[], char* short_opts_str)
{
    short_opts_str[0] = '-'; //Handle positional arguments in place
    short_opts_str[1] = ':'; //Cause ":" to be returned on missing arg
    int i = 1;

    for(srz_opt_t* opt = opts; !opt->fin; opt++){
        const char* srt = opt->srt;
        if(isempty(srt)){
            continue;
        }

        if(strlen(srt) > 1){
            SRZ_FAIL("%s (`%s`)\n", srz_err2str_en(SRZ_ERR_SOPT_TOO_LONG), srt);
            return SRZ_ERR_SOPT_TOO_LONG;
        }

        const char srt_opt = srt[0];

        if(strchr(short_opts_str, srt_opt)){
            SRZ_FAIL("%s `%c`\n", srz_err2str_en(SRZ_ERR_SOPT_DUP), srt_opt);
            return SRZ_ERR_SOPT_DUP;
        }

        if(SRZ_SOPTS_MAX - i < 2){
            SRZ_FAIL("%s. Current size = %i\n", srz_err2str_en(SRZ_ERR_SOPTS_MAX_TOO_SMALL), SRZ_SOPTS_MAX);
            return SRZ_ERR_SOPTS_MAX_TOO_SMALL;
        }

/* GNU supports optional short arguments as an extension */
#ifndef _GNU_SOURCE
        if(opt->atype == SRZ_ARG_OPT){
            SRZ_FAIL("%s '%c'\n", srz_err2str_en(SRZ_ERR_SOPT_OPTIONAL), srt_opt);
            return SRZ_ERR_SOPT_OPTIONAL;
        }
#endif

        short_opts_str[++i] = srt_opt;

        if(opt->atype == SRZ_ARG_REQ || opt->atype == SRZ_ARG_POS){

            if(SRZ_SOPTS_MAX - i < 2){
                SRZ_FAIL("%s. Current size = %i\n", srz_err2str_en(SRZ_ERR_SOPTS_MAX_TOO_SMALL), SRZ_SOPTS_MAX);
                return SRZ_ERR_SOPTS_MAX_TOO_SMALL;
            }


            short_opts_str[++i] = ':';
        }

/* GNU supports optional short arguments as an extension */
#ifdef _GNU_SOURCE
        if(opt->atype == ARG_OPT){

            if(SRZ_SOPTS_MAX - i < 2){
                SRZ_FAIL("%s. Current size = %i\n", srz_err2str_en(SRZ_ERR_SOPTS_MAX_TOO_SMALL), SRZ_SOPTS_MAX);
                return SRZ_ERR_SOPTS_MAX_TOO_SMALL;
            }

            short_opts_str[++i] = ':';
        }
#endif
    }

    return SRZ_ERR_NONE;
}

static inline srz_errno_t _srz_build_long_opts(srz_opt_t opts[], struct option* long_opts)
{
    int i = 0;

    for(srz_opt_t* opt = opts; !opt->fin; opt++, i++){
        const char* lng = opt->lng;

        if(isempty(lng)){
            continue;
        }

        if(_srz_find_long(opts, lng, &opt->ident)){
            SRZ_FAIL("%s `%s`\n", srz_err2str_en(SRZ_ERR_LOPT_DUP), lng);
            return SRZ_ERR_LOPT_DUP;
        }

        if(SRZ_LOPTS_MAX - i < 2){
            SRZ_FAIL("%s. Current size = %i\n", srz_err2str_en(SRZ_ERR_LOPTS_MAX_TOO_SMALL), SRZ_LOPTS_MAX);
            return SRZ_ERR_LOPTS_MAX_TOO_SMALL;
        }

        long_opts[i].name = opt->lng;
        long_opts[i].val  = 0;
        switch(opt->atype){
            case SRZ_ARG_NON:   long_opts[i].has_arg = no_argument;         break;
            case SRZ_ARG_OPT:   long_opts[i].has_arg = optional_argument;   break;
            case SRZ_ARG_REQ:   long_opts[i].has_arg = required_argument;   break;
            case SRZ_ARG_POS:   long_opts[i].has_arg = required_argument;   break;
        }

    }

    return SRZ_ERR_NONE;
}

void _srz_debug_dump_long(struct option* opts)
{
#ifdef SRZ_DEBUG
    int i = 0;
    for(struct option* opt = opts; opt->name; opt++, i++){
        printf("%i - name=%s, has_arg=%i, flag=%p, val=%i\n",
               i,
               opt->name,
               opt->has_arg,
               (void*)opt->flag,
               opt->val);
    }
#endif
}


srz_errno_t _srz_do_getop(
        int argc,
        char** argv,
        srz_opt_t opts[],
        srz_opt_handler_t opt_handler,
        char* short_opts_str,
        struct option* long_opts,
        void* user
    )
{
    srz_errno_t err = SRZ_ERR_NONE;

    int optindx = -1;
    int opt = -1;

    while(1){
        opt = getopt_long(argc, argv, short_opts_str, long_opts, &optindx);
        if(opt == -1){
            break;
        }

        srz_opt_t* srz_opt = NULL;
        srz_opt_type_t opt_type = SRZ_OPT_NONE;
        switch(opt){
            case 0:
                srz_opt = _srz_find_long(opts, long_opts[optindx].name, NULL);
                if(!srz_opt){
                    SRZ_FAIL("%s. (`%s`)\n", srz_err2str_en(SRZ_ERR_INTERNAL), long_opts[optindx].name);
                    return SRZ_ERR_INTERNAL;
                }
                opt_type = SRZ_OPT_LONG;
                break;
            case 1:
                srz_opt = _srz_get_positional(opts);
                if(!srz_opt){
                    SRZ_WARN("%s.\n", srz_err2str_en(SRZ_ERR_POSTIONAL_FOUND));
                    return SRZ_ERR_POSTIONAL_FOUND;
                }
                opt_type = SRZ_OPT_LONG;
                break;

            case '?':
                SRZ_DBG("Unknown option `%c` %s %i %s`\n", optopt, optarg, optind, argv[optind -1]);
                srz_opt = _srz_fuzzy_find_opt(opts,argv[optind -1], &opt_type);
                switch(opt_type){
                    case SRZ_OPT_NONE:
                        opt_type = SRZ_OPT_UNKOWN_NONE;
                        break;
                    case SRZ_OPT_SHORT:
                        opt_type = SRZ_OPT_UNKOWN_SHORT;
                        break;
                    case SRZ_OPT_LONG:
                        opt_type = SRZ_OPT_UNKOWN_LONG;
                        break;
                    default:
                        SRZ_FAIL("%s. (`%c`)\n", srz_err2str_en(SRZ_ERR_INTERNAL2), opt_type);
                        return SRZ_ERR_INTERNAL2;
                }
                break;

            case ':':
                SRZ_DBG("Missing argument for `%c` %s %i %s\n", optopt, optarg, optind, argv[optind -1]);
                srz_opt = _srz_fuzzy_find_opt(opts,argv[optind -1], &opt_type);

                switch(opt_type){
                    case SRZ_OPT_NONE:
                        opt_type = SRZ_OPT_ARG_MISSING_NONE;
                        break;
                    case SRZ_OPT_SHORT:
                        opt_type = SRZ_OPT_ARG_MISSING_SHORT;
                        break;
                    case SRZ_OPT_LONG:
                        opt_type = SRZ_OPT_ARG_MISSING_LONG;
                        break;
                    default:
                        SRZ_FAIL("%s. (`%c`)\n", srz_err2str_en(SRZ_ERR_INTERNAL2), opt_type);
                        return SRZ_ERR_INTERNAL2;
                }
                break;

            default:
                srz_opt = _srz_find_short(opts,opt);
                opt_type = SRZ_OPT_SHORT;
                if(!srz_opt){
                    SRZ_FAIL("%s. (`%c`)\n", srz_err2str_en(SRZ_ERR_INTERNAL), opt);
                    return SRZ_ERR_INTERNAL;
                }
        }

        err = opt_handler(opt_type, srz_opt, optarg, user);
        if(err){
            return err;
        }

    }

    srz_opt_t* srz_opt = _srz_get_positional(opts);
    if(optind < argc){
        if(!srz_opt){
            SRZ_WARN("%s.\n", srz_err2str_en(SRZ_ERR_POSTIONAL_FOUND));
            return SRZ_ERR_POSTIONAL_FOUND;
        }

        for(; optind < argc; optind++){
            err = opt_handler(SRZ_OPT_POS, srz_opt, argv[optind], user);
            if(err){
                return err;
            }
        }
    }

    return err;
}

srz_errno_t srz_parse_ex(int argc, char** argv, srz_opt_t* opts, srz_opt_handler_t opt_handler, void* user)
{

    srz_errno_t err = SRZ_ERR_NONE;
    if(_srz_no_short_long(opts)){
        SRZ_FAIL("%s.\n", srz_err2str_en(SRZ_ERR_NO_SHORT_LONG));
        return SRZ_ERR_NO_SHORT_LONG;
    }

    if(_srz_positional_count(opts) > 1){
        SRZ_FAIL("%s.\n", srz_err2str_en(SRZ_ERR_MULTI_POSITIONAL));
        return SRZ_ERR_MULTI_POSITIONAL;
    }

    char short_opts_str[SRZ_SOPTS_MAX];
    memset(short_opts_str, 0, SRZ_SOPTS_MAX);
    err = _srz_build_short_opts(opts, short_opts_str);
    if(err){
        SRZ_FAIL("Could not build short options string\n");
        return err;
    }

    struct option long_opts[SRZ_LOPTS_MAX];
    memset(&long_opts, 0, sizeof(struct option) * SRZ_LOPTS_MAX);
    err = _srz_build_long_opts(opts, long_opts);
    if(err){
        SRZ_FAIL("Could not build long options structure\n");
        return err;
    }

    _srz_debug_dump_long(long_opts);

    _srz_do_getop(argc,argv,opts,opt_handler,short_opts_str,long_opts, user);

    return err;
}




/* Implicit SRZ state holder */
srz_t ___srz___;

static inline void _srz_init(void)
{
    if(!___srz___.init_complete){
        memset(&___srz___, 0, sizeof(srz_t));
        ___srz___.init_complete = 1;
    }
}

static inline int _srz_add_check()
{
    if(SRZ_OPTS_MAX - ___srz___.opt_idx < 2){
        SRZ_FAIL("%s. Current size = %i\n", srz_err2str_en(SRZ_ERR_SOPTS_MAX_TOO_SMALL), SRZ_SOPTS_MAX);
        return SRZ_ERR_SOPTS_MAX_TOO_SMALL;
    }

    return 0;
}


#define _srz_add_x_imp(I,N,T,O)                             \
_srz_add_x(N,T)                                             \
{                                                           \
    _srz_init();                                            \
                                                            \
    if(_srz_add_check()){                                   \
        ___srz___.errno = SRZ_ERR_SOPTS_MAX_TOO_SMALL;      \
        return -1 ;                                         \
    }                                                       \
                                                            \
    srz_opt_t* opt  = ___srz___.opts + ___srz___.opt_idx;   \
    opt->fin        = 0;                                    \
    opt->ident      = ___srz___.opt_idx;                    \
    opt->srt        = sopt;                                 \
    opt->lng        = lopt;                                 \
    opt->desc       = desc;                                 \
    opt->atype      = SRZ_ARG_REQ;                          \
    opt->val.type   = O;                                    \
    opt->val.init.I = init;                                 \
    opt->val.dest   = dest;                                 \
    opt->val.is_vector = 0;                                 \
                                                            \
    *dest = init;                                           \
                                                            \
    ___srz___.opt_idx++;                                    \
    opt = ___srz___.opts + ___srz___.opt_idx;               \
    opt->fin = 1;                                           \
                                                            \
    return 0;                                               \
}

_srz_add_x_imp(i, i,   int,      SRZ_VAL_INT)
_srz_add_x_imp(i, i8,  int8_t,   SRZ_VAL_INT8)
_srz_add_x_imp(i, i16, int16_t,  SRZ_VAL_INT16)
_srz_add_x_imp(i, i32, int32_t,  SRZ_VAL_INT32)
_srz_add_x_imp(i, i64, int64_t,  SRZ_VAL_INT64)
_srz_add_x_imp(i, b,   bool,     SRZ_VAL_BOOL)
_srz_add_x_imp(u, u,   unsigned, SRZ_VAL_INT)
_srz_add_x_imp(u, u8,  uint8_t,  SRZ_VAL_UINT8)
_srz_add_x_imp(u, u16, uint16_t, SRZ_VAL_UINT16)
_srz_add_x_imp(u, u32, uint32_t, SRZ_VAL_UINT32)
_srz_add_x_imp(u, u64, uint64_t, SRZ_VAL_UINT64)
_srz_add_x_imp(f, f,   float,    SRZ_VAL_FLOAT)
_srz_add_x_imp(f, d,   double,   SRZ_VAL_DOUBLE)
_srz_add_x_imp(s, s,   char*,    SRZ_VAL_STR)

#define _srz_add_X_imp(N,T, O)                              \
_srz_add_X(N,T)                                             \
{                                                           \
    _srz_init();                                            \
                                                            \
    if(_srz_add_check()){                                   \
        ___srz___.errno = SRZ_ERR_SOPTS_MAX_TOO_SMALL;      \
        return -1 ;                                         \
    }                                                       \
                                                            \
    srz_opt_t* opt = ___srz___.opts + ___srz___.opt_idx;    \
    opt->ident = ___srz___.opt_idx;                         \
    opt->fin   = 0;                                         \
    opt->srt   = sopt;                                      \
    opt->lng   = lopt;                                      \
    opt->desc  = desc;                                      \
    opt->atype = SRZ_ARG_REQ;                               \
    opt->val.type = O;                                      \
    opt->val.dest = dest;                                   \
    opt->val.is_vector = 1;                                 \
                                                            \
    ___srz___.opt_idx++;                                    \
    opt = ___srz___.opts + ___srz___.opt_idx;               \
    opt->fin = 1;                                           \
                                                            \
    return 0;                                               \
}

_srz_add_X_imp(I,   int,      SRZ_VAL_INT)
_srz_add_X_imp(I8,  int8_t,   SRZ_VAL_INT8)
_srz_add_X_imp(I16, int16_t,  SRZ_VAL_INT16)
_srz_add_X_imp(I32, int32_t,  SRZ_VAL_INT32)
_srz_add_X_imp(I64, int64_t,  SRZ_VAL_INT64)
_srz_add_X_imp(B,   bool,     SRZ_VAL_BOOL)
_srz_add_X_imp(U,   unsigned, SRZ_VAL_UINT)
_srz_add_X_imp(U8,  uint8_t,  SRZ_VAL_UINT8)
_srz_add_X_imp(U16, uint16_t, SRZ_VAL_UINT16)
_srz_add_X_imp(U32, uint32_t, SRZ_VAL_UINT32)
_srz_add_X_imp(U64, uint64_t, SRZ_VAL_UINT64)
_srz_add_X_imp(F,   float,    SRZ_VAL_FLOAT)
_srz_add_X_imp(D,   double,   SRZ_VAL_DOUBLE)
_srz_add_X_imp(S,   char*,    SRZ_VAL_STR)


int srz_add_P(char* sopt, char* lopt, char* desc, char*** dest)
{
    _srz_init();

    if(_srz_add_check()){
        ___srz___.errno = SRZ_ERR_SOPTS_MAX_TOO_SMALL;
        return -1 ;
    }

    srz_opt_t* opt = ___srz___.opts + ___srz___.opt_idx;
    opt->ident          = ___srz___.opt_idx;
    opt->fin            = 0;
    opt->srt            = sopt;
    opt->lng            = lopt;
    opt->desc           = desc;
    opt->atype          = SRZ_ARG_POS;
    opt->val.type       = SRZ_VAL_STR;
    opt->val.dest       = dest;
    opt->val.is_vector  = 1;

    ___srz___.opt_idx++;
    opt = ___srz___.opts + ___srz___.opt_idx;
    opt->fin = 1;

    return 0;
}


int srz_add_e(char* sopt, char* lopt, char* desc, int* dest, int init, srz_enum_t* map)
{
    _srz_init();

    if(_srz_add_check()){
        ___srz___.errno = SRZ_ERR_SOPTS_MAX_TOO_SMALL;
        return -1 ;
    }

    srz_opt_t* opt = ___srz___.opts + ___srz___.opt_idx;
    opt->ident          = ___srz___.opt_idx;
    opt->fin            = 0;
    opt->srt            = sopt;
    opt->lng            = lopt;
    opt->desc           = desc;
    opt->atype          = SRZ_ARG_REQ;
    opt->val.type       = SRZ_VAL_ENUM;
    opt->val.dest       = dest;
    opt->val.enm_map    = map;
    opt->val.init.i     = init;
    opt->val.is_vector  = 0;

    ___srz___.opt_idx++;
    opt = ___srz___.opts + ___srz___.opt_idx;
    opt->fin = 1;

    return 0;
}

int srz_add_E(char* sopt, char* lopt, char* desc, int** dest, srz_enum_t* map)
{
    _srz_init();

    if(_srz_add_check()){
        ___srz___.errno = SRZ_ERR_SOPTS_MAX_TOO_SMALL;
        return -1 ;
    }

    srz_opt_t* opt = ___srz___.opts + ___srz___.opt_idx;
    opt->ident          = ___srz___.opt_idx;
    opt->srt            = sopt;
    opt->lng            = lopt;
    opt->desc           = desc;
    opt->atype          = SRZ_ARG_REQ;
    opt->val.type       = SRZ_VAL_ENUM;
    opt->val.dest       = dest;
    opt->val.enm_map    = map;
    opt->val.is_vector  = 1;

    ___srz___.opt_idx++;
    opt = ___srz___.opts + ___srz___.opt_idx;
    opt->fin = 1;

    return 0;
}

static inline char* _srz_opt_type2str(srz_opt_type_t opt_type)
{
    switch(opt_type){
        case SRZ_OPT_NONE:
            return "none";
        case SRZ_OPT_SHORT:
            return "short";
        case SRZ_OPT_POS:
            return "positional";
        case SRZ_OPT_ARG_MISSING_NONE:
            return "arg missing - none";
        case SRZ_OPT_ARG_MISSING_SHORT:
            return "arg missing - short";
        case SRZ_OPT_ARG_MISSING_LONG:
            return "missing - long";
        case SRZ_OPT_UNKOWN_NONE:
            return "unknown - none";
        case SRZ_OPT_UNKOWN_SHORT:
            return "unknown - short";
        case SRZ_OPT_UNKOWN_LONG:
            return "unknown - long";
        default:
            return NULL;
    }
}

static int _srz_opt_handler(srz_opt_type_t opt_type, const srz_opt_t* const opt, const char* const optval, void* user)
{
    (void)user;

    printf("got option of type %s\n", _srz_opt_type2str(opt_type));
    char* opt_name = NULL;
    switch(opt_type){
        case SRZ_OPT_SHORT:
        case SRZ_OPT_ARG_MISSING_SHORT:
        case SRZ_OPT_UNKOWN_SHORT:
            opt_name = opt->srt;
            break;
        case SRZ_OPT_LONG:
        case SRZ_OPT_ARG_MISSING_LONG:
        case SRZ_OPT_UNKOWN_LONG:
            opt_name = opt->lng;
            break;
        case SRZ_OPT_NONE:
        case SRZ_OPT_ARG_MISSING_NONE:
        case SRZ_OPT_UNKOWN_NONE:
            opt_name = "[unkown]";
            break;
        case SRZ_OPT_POS:
            opt_name = "";

    }

    if(opt){
        switch(opt->atype){
            case SRZ_ARG_REQ:
                printf("Option %s with required arg: %s\n", opt_name, optval);
                break;
            case SRZ_ARG_NON:
                printf("Option %s with no arg\n", opt_name);
                break;
            case SRZ_ARG_OPT:
                printf("Option %s with optional arg: %s", opt_name, optval ? optval : "(none)");
                break;
            case SRZ_ARG_POS:
                printf("Positional options %s %s\n", opt_name, optval);
                break;
        }
    }

    return 0;
}


int srz_parse(int argc, char** argv)
{
    if(!___srz___.init_complete){
        ___srz___.errno = SRZ_ERR_NO_OPTS_ADDED;
        return -1;
    }

    ___srz___.errno = srz_parse_ex(argc,argv,___srz___.opts,_srz_opt_handler,NULL);
    if(___srz___.errno != SRZ_ERR_NONE){
        return -1;
    }

    return 0;
}


#endif /* SRZ_HONLY */
#endif /* SSRZH_ */
