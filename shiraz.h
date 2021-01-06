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
    ARG_NON, //Argument is not expected
    ARG_OPT, //Argument is optional
    ARG_REQ, //Argument is required
} srz_atype_t;

typedef struct srz_opt {
    int64_t ident;
    srz_atype_t atype;
    char* srt;
    char* lng;
    char* descr;
    bool fin;
    int64_t count;
} srz_opt_t;

#define SRZ_FIN { .fin = true }
#define SRZ_OPT(IDENT, ATYPE, SHORT, LONG, DESCR) \
{ \
    .ident    = IDENT,                     \
    .atype    = ATYPE,                     \
    .srt      = SHORT,                     \
    .lng      = LONG,                      \
    .descr    = DESCR,                     \
    .fin      = false,                     \
    .count    = 0,                         \
}

//Remember to update the string translation table srz_error_en[]
typedef enum {
    SRZ_ERR_NONE = 0,
    SRZ_ERR_SOPTS_MAX_TOO_SMALL,
    SRZ_ERR_SHORT_OPT_DUP,
    SRZ_ERR_SHORT_OPT_OPTIONAL,
} srz_errno_t;

typedef struct srz_error_en {
    srz_errno_t err_no;
    const char* err_str;
} srz_err_t;


typedef int (* srz_opt_handler_t)(int optnum, char* optval);

srz_errno_t srz_parseopts(int argc, char** argv, srz_opt_t opts[], srz_opt_handler_t opt_handler);


/*
* Implementation!
* ============================================================================
*/
#ifndef SRZ_HONLY

static struct srz_error_en srz_error_en[] = {
    { SRZ_ERR_NONE,                 "No error"},
    { SRZ_ERR_SOPTS_MAX_TOO_SMALL,  "The short options string space is too small, try enlarging SRZ_SOPTS_MAX and recompiling" },
    { SRZ_ERR_SHORT_OPT_DUP,        "Duplicate short options entry. Remove or rename this short option name" },
    { SRZ_ERR_SHORT_OPT_OPTIONAL,   "A short option cannot have an optional argument. Remove short option or use ARG_NON or ARG_REQ argument type"},
    { 0, 0 }
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
    }
    fprintf(stderr, "[%s %s:%i:%s()]  ", mode_str, basename((char*)fn), ln, fu);
    vfprintf(stderr, msg, args);

    if(mode == SRZ_MSG_ERR && SRZ_HARD_EXIT){
        exit(0xDEAD);
    }
    va_end(args);
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

static inline srz_opt_t* _srz_find_long(srz_opt_t opts[], char* l) {
    for(srz_opt_t* opt = opts; !opt->fin; opt++){
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


static inline srz_errno_t _srz_build_short_opts(srz_opt_t opts[], char* short_opts_str) {
    short_opts_str[0] = ':'; //Cause ":" to be returned on missing arg
    int i = 0;

    for(srz_opt_t* opt = opts; !opt->fin; opt++){
        const char* srt = opt->srt;
        if(isempty(srt)){
            continue;
        }

        if(strlen(srt) > 1){
            continue; //This is really a long option in disguise
        }

        const char srt_opt = srt[0];

        if(strchr(short_opts_str,srt_opt)){
            SRZ_FAIL("%s `%c`\n", srz_err2str_en(SRZ_ERR_SHORT_OPT_DUP), srt_opt);
            return SRZ_ERR_SHORT_OPT_DUP;
        }

        if(SRZ_SOPTS_MAX - i < 3){
            SRZ_FAIL("%s. Current size = %i\n", srz_err2str_en(SRZ_ERR_SOPTS_MAX_TOO_SMALL), SRZ_SOPTS_MAX);
            return SRZ_ERR_SOPTS_MAX_TOO_SMALL;
        }

        if(opt->atype == ARG_OPT){
            SRZ_FAIL("%s '%c'\n", srz_err2str_en(SRZ_ERR_SHORT_OPT_OPTIONAL), srt_opt);
            return SRZ_ERR_SHORT_OPT_OPTIONAL;
        }

        short_opts_str[++i] = srt_opt;
        
        if(opt->atype == ARG_REQ){
            short_opts_str[++i] = ':';
        }

    }

    printf("%s\n", short_opts_str);

    return SRZ_ERR_NONE;
}

static inline srz_errno_t _srz_build_long_opts(srz_opt_t opts[], struct option* long_opts)
{
    (void)opts;
    (void)long_opts;
    return SRZ_ERR_NONE;
}

srz_errno_t srz_parseopts(int argc, char** argv, srz_opt_t opts[], srz_opt_handler_t opt_handler) {

    (void)argc;
    (void)argv;
    (void)opt_handler;
    srz_errno_t err = SRZ_ERR_NONE;
    
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

    
    return err;
}


#endif /* SRZ_HONLY */


#endif /* SSRZH_ */
