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
 * DISCLAIMED. IN NO SRZENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWSRZER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, SRZEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#define SRZ_SOPTS_SMAX 256 //Maximum number of charters to use for short options

/*
 * Forward declarations of the the SRZ interface functions
 * ===========================================================================
 */


#if SRZ_PEDANTIC
    #define ifp(p,e) if(p){e}
#else
    #define ifp(p,e)
#endif

typedef enum {
    SRZ_ARG_NON, //Argument is not expected
    SRZ_ARG_OPT, //Argument is optional
    SRZ_ARG_REQ, //Argument is required
} srz_ptype_t;

typedef struct srz_opt {
    int64_t opt_num;

    char* opt_short;
    char* opt_long;
    char* opt_descr;
    bool fin;
    int64_t count;
} srz_opt_t ;

#define SRZ_FIN { .fin = true }
#define SRZ_OPT(IDENT, SHORT, LONG, DESCR) \
{ \
    .opt_num    = IDENT, \
    .opt_short  = SHORT, \
    .opt_long   = LONG,  \
    .opt_descr  = DESCR, \
    .fin        = false, \
    .count      = 0,     \
}

typedef int (*srz_opt_hanlder_t)(int optnum, char* optval);

int srz_parseopts(int argc, char** argv, srz_opt_t opts[], srz_opt_hanlder_t opt_hanlder);


/*
* Implementation!
* ============================================================================
*/
#ifndef SRZ_HONLY

/*
 * This debugging code liberally borrowed and adapted from libchaste by
 * M.Grosvenor BSD 3 clause license. https://github.com/mgrosvenor/libchaste
 */
typedef enum {
    SRZ_MSG_DBG,
    SRZ_MSG_WARN,
    SRZ_MSG_ERR,
} srz_dbg_e;

#define SRZ_FAIL( /*format, args*/...)  srz_err_helper(__VA_ARGS__, "")
#define srz_err_helper(format, ...) _srz_msg(SRZ_MSG_ERR, __LINE__, __FILE__, __FUNCTION__, format, __VA_ARGS__ )
#define SRZ_WARN( /*format, args*/...)  srz_warn_helper(__VA_ARGS__, "")
#define srz_warn_helper(format, ...) _srz_msg(SRZ_MSG_WARN,__LINE__, __FILE__, __FUNCTION__, format, __VA_ARGS__ )

#if SRZ_DEBUG
    #define SRZ_DBG( /*format, args*/...)  srz_debug_helper(__VA_ARGS__, "")
    #define srz_debug_helper(format, ...) _srz_msg(SRZ_MSG_DBG,__LINE__, __FILE__, __FUNCTION__, format, __VA_ARGS__ )
#else
    #define SRZ_DBG( /*format, args*/...)
#endif

__attribute__ ((format (printf, 5, 6)))
static inline void _srz_msg(srz_dbg_e mode, int ln, char* fn, const char* fu, const char* msg, ... )
{
    va_list args;
    va_start(args,msg);
    char* mode_str = NULL;
    switch(mode){
        case SRZ_MSG_ERR:   mode_str = "Error  :"; break;
        case SRZ_MSG_DBG:   mode_str = "Debug  :"; break;
        case SRZ_MSG_WARN:  mode_str = "Warning:"; break;
    }
    dprintf(STDERR_FILENO,"[%s %s:%i:%s()]  ", mode_str, basename(fn), ln, fu);
    vdprintf(STDERR_FILENO,msg,args);

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


static int _srz_build_short_opts(srz_opt_t opts[], char* short_opts_str)
{
    //short_opts_str[0] = ":"; //Cause ":" to be returned
    //int i = 1;


    for(srz_opt_t* opt = opts; !opt->fin; opt++){
        const char* sopt_str = opt->opt_short;
        if(isempty(sopt_str)){
            continue;
        }

        if(strlen(sopt_str) > 1){
            continue; //This is really a long option in disguise
        }

        printf("Short=%s\n", opt->opt_short);
    }

    return 0;
}

int srz_parseopts(int argc, char** argv, srz_opt_t opts[], srz_opt_hanlder_t opt_hanlder)
{

    char short_opts_str[SRZ_SOPTS_SMAX] = {0};
    _srz_build_short_opts(opts, short_opts_str);

    return 0;
}



#endif /* SRZ_HONLY */



#endif /* SSRZH_ */
