#include <stdio.h>
#include "shiraz.h"

typedef enum {
    LOGGING,
    LOG_LEVEL,
    INTERFACES,
    OUTPUTS,
    COLOR,
    OOMPAS
} opt_t;

srz_opt_t opts[] = {
        SRZ_OPT( LOGGING,      "l", "logging",     "set the log path" ),
        SRZ_OPT( LOG_LEVEL,    "",  "log-level",   "set the log level [error, warning, info, debug]" ),
        SRZ_OPT( COLOR,       NULL, "colour",      "set the log colour "),
        SRZ_OPT( INTERFACES,   "int", "",          "interface to listen on, may be exanic or system interface" ),
        SRZ_OPT( OUTPUTS,      "o", NULL,          "output file(s) to write to" ),
        SRZ_OPT( OOMPAS,       "o", NULL,          "Number of OOMPAS" ),
        SRZ_FIN
};


int handle_opt(int optnm, char* optval )
{
    switch((opt_t)optnm){
        case LOGGING: break;
        case LOG_LEVEL: break;
        case INTERFACES: break;
        case OUTPUTS: break;
        case COLOR: break;
        case OOMPAS: break;
    }

    return 0;
}
int main( int argc, char** argv)
{

    srz_parseopts(argc, argv, opts, handle_opt);
    printf("This is a demo!\n");
#ifdef _GNU_SOURCE
    printf("GNUSource!\n");
#endif
    return 0;
}
