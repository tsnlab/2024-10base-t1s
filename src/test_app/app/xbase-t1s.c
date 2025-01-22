#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command_parser.h"
#include "log.h"

int verbose = 0;
char* progname = NULL;

int main(int argc, char* argv[]) {

    /* save program name */
    progname = strrchr(argv[0], '/');
    progname = ((!progname) ? argv[0] : progname + 1);

    /* setup log */
    log_init(progname, 0, 0);

    argc--, argv++;
    return command_parser(argc, argv);
}
