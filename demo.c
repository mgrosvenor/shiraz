#include <stdio.h>

#define SRZ_DEBUG
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
        SRZ_REQ(LOGGING, "l", "logging", "set the log path" ),
        SRZ_REQ(LOG_LEVEL, "", "log-level", "set the log level [error, warning, info, debug]" ),
        SRZ_FLG( COLOR,       "",  "colour",    "set the log colour "),
        SRZ_OPT( INTERFACES,  "", "interfaces",          "interface to listen on, may be exanic or system interface" ),
        SRZ_FLG( OUTPUTS,     "o", NULL,        "output file(s) to write to" ),
        SRZ_POS( OOMPAS,      "M", NULL,        "Number of OOMPAS" ),

        SRZ_FIN
};


int handle_opt(const srz_opt_t* const opt, const char* const optval, void* user)
{
    (void)user;
    switch(opt->ident){
        case LOGGING:       printf("Logging:");     break;
        case LOG_LEVEL:     printf("Log-level:");   break;
        case INTERFACES:    printf("Interfaces:");  break;
        case OUTPUTS:       printf("Outputs:");     break;
        case COLOR:         printf("Color:");       break;
        case OOMPAS:        printf("OOMPAS:");      break;
    }

    if(optval) {
        printf("%s", optval);
    }
    printf("\n");
    return 0;
}
int main( int argc, char** argv)
{

    int a = 1;
    srz_parseopts(argc, argv, opts, handle_opt, &a);

    return 0;
}
