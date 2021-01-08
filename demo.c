#include <stdio.h>

#define SRZ_DEBUG
#include "shiraz.h"


enum {
    RED,
    GREEN,
    BLUE,
    BLACK,
};

srz_enum_t test_e_map[] = {
    {RED,   "RED"},
    {GREEN, "GREEN"},
    {BLUE,  "BLUE"},
};


int main( int argc, char** argv)
{

    int flg = 1;
    char* str;
    char** strs;
    char** pos;
    int enm;
    int* enms;

    srz_opt("l", "logging",     "set the log path",     &str, ""                );
    srz_vec("L", "loggings",     "set the log path",     &strs                   );
    srz_flg("i", "interfaces",  "set the interface",    &flg                    );
    srz_pos("s", "positionals", "some positionals",     &pos                    );
    srz_enm("e", "enum",        "an enum",              &enm,  RED, test_e_map  );
    srz_ens("E", "enums",       "enums",                &enms, test_e_map       );

    srz_parse(argc, argv);

    return 0;
}
