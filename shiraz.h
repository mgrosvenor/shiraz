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
    SRZ_OPT_BOOL,
    SRZ_OPT_INT,
    SRZ_OPT_UINT,
    SRZ_OPT_FLOAT,
    SRZ_OPT_STR,
    SRZ_OPT_ENUM,
} srz_otype_t;

typedef struct srz_enum {
    int val;
    char* str;
} srz_enum_t;

typedef struct srz_val {
    srz_otype_t type;
    union {
        bool b;
        int64_t i;
        uint64_t u;
        double f;
        char* s;
    } init;

    union {
        bool* b;
        int64_t* i;
        uint64_t* u;
        double* f;
        char** s;

        //For vector types
        bool** B;
        int64_t** I;
        uint64_t** U;
        double** F;
        char*** S;
    } dest;

    srz_enum_t* enm;
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
    bool fin;
    srz_val_t val;
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
    SRZ_ERR_MULTI_POSITIONAL,
    SRZ_ERR_POSTIONAL_FOUND,
    SRZ_ERR_NO_SHORT_LONG,
    SRZ_ERR_OPTS_MAX_TOO_SMALL,
} srz_errno_t;

typedef struct srz_error_en {
    srz_errno_t err_no;
    const char* err_str;
} srz_err_t;


typedef int (* srz_opt_handler_t)(const srz_opt_t* const opt, const char* const optval, void* user);
srz_errno_t srz_parse(int argc, char** argv, srz_opt_t* opts, srz_opt_handler_t opt_handler, void* user);

#define _srz_add_x(n,T) \
    int srz_add_##n(char* sopt, char* lopt, char* desc, T* dest, T init)

#define _srz_add_X(N,T) \
    int srz_add_##N(char* sopt, char* lopt, char* desc, T** dest)

#define _srz_add_xX(n,N,T) \
    _srz_add_x(n,T); \
    _srz_add_X(N,T)

_srz_add_xX(i, I, int64_t);
_srz_add_xX(u, U, uint64_t);
_srz_add_xX(f, F, double);
_srz_add_xX(s, S, char*);

#if __STDC__==1 && __STDC_VERSION__ >= 201112L
#define srz_opt(sopt, lopt, descr, dest, init) _Generic( (init),   \
              int8_t: srz_add_i,                                   \
              int16_t: srz_add_i,                                  \
              int32_t: srz_add_i,                                  \
              int64_t: srz_add_i,                                  \
              uint8_t: srz_add_u,                                  \
              uint16_t: srz_add_u,                                 \
              uint32_t: srz_add_u,                                 \
              uint64_t: srz_add_u,                                 \
              float: srz_add_f,                                    \
              double: srz_add_f,                                   \
              char*: srz_add_s                                     \
              )( sopt, lopt, descr, dest, init)

#define srz_vec(sopt, lopt, descr, dest) _Generic( (dest),   \
              int8_t**: srz_add_I,                           \
              int16_t**: srz_add_I,                          \
              int32_t**: srz_add_I,                          \
              int64_t**: srz_add_I,                          \
              uint8_t**: srz_add_U,                          \
              uint16_t**: srz_add_U,                         \
              uint32_t**: srz_add_U,                         \
              uint64_t**: srz_add_U,                         \
              float**: srz_add_F,                            \
              double**: srz_add_F,                           \
              char***: srz_add_S                             \
              )( sopt, lopt, descr, dest)
#endif


int srz_add_e(char* sopt, char* lopt, char* desc, int* dest, int init, srz_enum_t* map);
int srz_add_E(char* sopt, char* lopt, char* desc, int** dest, srz_enum_t* map);
#define srz_enm(sopt, lopt, desc, dest, init, map) \
    srz_add_e(sopt, lopt, desc, dest, init, map)
#define srz_ens(sopt, lopt, desc, dest, map) \
    srz_add_E(sopt, lopt, desc, dest, map)


int srz_add_b( char* sopt, char* lopt, char* desc, int* dest);
#define srz_flg(sopt, lopt, desc, dest) \
    srz_add_b(sopt, lopt, desc, dest)

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
    {SRZ_ERR_MULTI_POSITIONAL,    "Multiple positional options found (ARG_POS), only 1 permitted. Remove additional options"},
    {SRZ_ERR_POSTIONAL_FOUND,     "Positional arguments found, but there is no positional option. Add ARG_POS, or remove positional arguments"},
    {SRZ_ERR_NO_SHORT_LONG,       "Neither a short or long option string were supplied. Both cannot be blank. Either a short or long option string is required"},
    {SRZ_ERR_OPTS_MAX_TOO_SMALL,  "The options memory space is too small, reduce the number of options in use or enlarge SRZ_OPTS_MAX and recompile" },
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
static inline void _srz_msg(srz_dbg_e mode, int ln, const char* fn, const char* fu, const char* msg, ...) {
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
    if(SRZ_OPTS_MAX - ___srz___.opt_idx < 1){
        SRZ_FAIL("%s. Current size = %i\n", srz_err2str_en(SRZ_ERR_SOPTS_MAX_TOO_SMALL), SRZ_SOPTS_MAX);
        return SRZ_ERR_SOPTS_MAX_TOO_SMALL;
    }

    return 0;
}


#define _srz_add_x_imp(N,T,O)                               \
_srz_add_x(N,T)                                             \
{                                                           \
    _srz_init();                                            \
                                                            \
    if(_srz_add_check()){                                   \
        return SRZ_ERR_SOPTS_MAX_TOO_SMALL;                 \
    }                                                       \
                                                            \
    srz_opt_t* opt = ___srz___.opts + ___srz___.opt_idx;    \
    opt->ident = ___srz___.opt_idx;                         \
    opt->srt   = sopt;                                      \
    opt->lng   = lopt;                                      \
    opt->desc  = desc;                                      \
    opt->atype = SRZ_ARG_REQ;                               \
    opt->val.type = O;                                      \
    opt->val.init.N = init;                                 \
    opt->val.dest.N = dest;                                 \
                                                            \
    *dest = init;                                           \
                                                            \
    ___srz___.opt_idx++;                                    \
                                                            \
    return 0;                                               \
}

_srz_add_x_imp(i,int64_t,  SRZ_OPT_INT)
_srz_add_x_imp(u,uint64_t, SRZ_OPT_UINT)
_srz_add_x_imp(f,double,   SRZ_OPT_FLOAT)
_srz_add_x_imp(s,char*,    SRZ_OPT_STR)


#define _srz_add_X_imp(N,T, O)                              \
_srz_add_X(N,T)                                             \
{                                                           \
    _srz_init();                                            \
                                                            \
    if(_srz_add_check()){                                   \
        return SRZ_ERR_SOPTS_MAX_TOO_SMALL;                 \
    }                                                       \
                                                            \
    srz_opt_t* opt = ___srz___.opts + ___srz___.opt_idx;    \
    opt->ident = ___srz___.opt_idx;                         \
    opt->srt   = sopt;                                      \
    opt->lng   = lopt;                                      \
    opt->desc  = desc;                                      \
    opt->atype = SRZ_ARG_REQ;                               \
    opt->val.type = O;                                      \
    opt->val.dest.N = dest;                                 \
    ___srz___.opt_idx++;                                    \
                                                            \
    return 0;                                               \
}

_srz_add_X_imp(I,int64_t,  SRZ_OPT_INT)
_srz_add_X_imp(U,uint64_t, SRZ_OPT_UINT)
_srz_add_X_imp(F,double,   SRZ_OPT_FLOAT)
_srz_add_X_imp(S,char*,    SRZ_OPT_STR)


int srz_add_E(char* sopt, char* lopt, char* desc, int** dest, srz_enum_t* map)
{
    return 0;
}


int srz_add_b( char* sopt, char* lopt, char* desc, int* dest)
{
    return 0;
}


int srz_add_P(char* sopt, char* lopt, char* desc, char*** dest)
{
    return 0;
}


int srz_add_e(char* sopt, char* lopt, char* desc, int* dest, int init, srz_enum_t* map)
{
    _srz_init();

    if(_srz_add_check()){
        return SRZ_ERR_SOPTS_MAX_TOO_SMALL;
    }

    srz_opt_t* opt = ___srz___.opts + ___srz___.opt_idx;
    opt->ident = ___srz___.opt_idx;
    opt->srt   = sopt;
    opt->lng   = lopt;
    opt->desc  = desc;
    opt->atype = SRZ_ARG_REQ;
    opt->val.type = O;
    opt->val.dest.N = dest;

    ___srz___.opt_idx++;

    return 0;
}

static inline int isempty(const char* s) {
    if(s == NULL){
        return 1;
    }
    if(strlen(s) == 0){
        return 1;
    }

    return 0;
}


static inline srz_opt_t* _srz_find_short(srz_opt_t opts[], char s) {
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

static inline srz_opt_t* _srz_find_long(srz_opt_t opts[], const char* l, int* ignore) {
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

static inline int _srz_positional_count(srz_opt_t opts[]) {
    int result = 0;
    for(srz_opt_t* opt = opts; !opt->fin; opt++){

        if(opt->atype == SRZ_ARG_POS){
            result++;
        }
    }

    return result;
}

static inline srz_opt_t* _srz_get_positional(srz_opt_t opts[]) {
    for(srz_opt_t* opt = opts; !opt->fin; opt++){

        if(opt->atype == SRZ_ARG_POS){
            return opt;
        }
    }

    return NULL;
}

static inline int _srz_no_short_long(srz_opt_t opts[]) {
    for(srz_opt_t* opt = opts; !opt->fin; opt++){
        if(opt->atype != SRZ_ARG_POS){
            if(isempty(opt->srt) && isempty(opt->lng)){
                return 1;
            }
        }
    }

    return 0;
}


static inline srz_errno_t _srz_build_short_opts(srz_opt_t opts[], char* short_opts_str) {
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

    SRZ_DBG("%s\n", short_opts_str);

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
    srz_opt_t* srz_opt = NULL;

    while(1){
        opt = getopt_long(argc, argv, short_opts_str, long_opts, &optindx);
        if(opt == -1){
            break;
        }

        switch(opt){
            case 0:
                srz_opt = _srz_find_long(opts, long_opts[optindx].name, NULL);
                if(!srz_opt){
                    SRZ_FAIL("%s. (`%s`)\n", srz_err2str_en(SRZ_ERR_INTERNAL), long_opts[optindx].name);
                    return SRZ_ERR_INTERNAL;
                }
                break;
            case 1:
                srz_opt = _srz_get_positional(opts);
                if(!srz_opt){
                    SRZ_WARN("%s.\n", srz_err2str_en(SRZ_ERR_POSTIONAL_FOUND));
                    return SRZ_ERR_POSTIONAL_FOUND;
                }
                break;

            case '?':
                printf("Unknown option `%c` %s %i %s`\n", optopt, optarg, optind, argv[optind -1]);
                continue;

            case ':':
                printf("Missing option for `%c` %s %i %s\n", optopt, optarg, optind, argv[optind -1]);
                continue;

            default:
                srz_opt = _srz_find_short(opts,opt);
                if(!srz_opt){
                    SRZ_FAIL("%s. (`%c`)\n", srz_err2str_en(SRZ_ERR_INTERNAL), opt);
                    return SRZ_ERR_INTERNAL;
                }
        }

        err = opt_handler(srz_opt, optarg, user);
        if(err){
            return err;
        }

    }


    srz_opt = _srz_get_positional(opts);
    if(optind < argc){
        if(!srz_opt){
            SRZ_WARN("%s.\n", srz_err2str_en(SRZ_ERR_POSTIONAL_FOUND));
            return SRZ_ERR_POSTIONAL_FOUND;
        }

        for(; optind < argc; optind++){
            err = opt_handler(srz_opt, argv[optind], user);
            if(err){
                return err;
            }
        }
    }



    return err;
}

srz_errno_t srz_parse(int argc, char** argv, srz_opt_t* opts, srz_opt_handler_t opt_handler, void* user) {


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


#endif /* SRZ_HONLY */


#endif /* SSRZH_ */
