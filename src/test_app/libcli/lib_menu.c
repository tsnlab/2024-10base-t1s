#include "lib_menu.h"

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/ethernet.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/types.h>

#include "error_define.h"
#include "libcom.h"

void show_menuitem_in_details(menu_command_t* menu_tbl, int index) {
    if (menu_tbl[index].name == NULL) {
        return;
    }

    if (menu_tbl[index].attr == DIRECTORY_ATTR) {
        printf(ANSI_COLOR_BLUE "[DIR] %-12s\n", menu_tbl[index].name);
    } else {
        printf(ANSI_COLOR_RESET "[SUB-CMD] %-12s\n", menu_tbl[index].name);
    }

    printf(ANSI_COLOR_BLUE "SYNOPSIS\n");
    printf(ANSI_COLOR_RESET "%s\n", menu_tbl[index].usage);
    printf(ANSI_COLOR_BLUE "DESCRIPTION\n");
    printf(ANSI_COLOR_RESET "%s\n", menu_tbl[index].desc);
}

void show_menu(menu_command_t* menu_tbl) {
    int i;

    for (i = 0; menu_tbl[i].name != NULL; i++) {
        if (menu_tbl[i].attr == DIRECTORY_ATTR) {
            printf(ANSI_COLOR_BLUE "  %-12s\n", menu_tbl[i].name);
        }
    }

    for (i = 0; menu_tbl[i].name != NULL; i++) {
        if (menu_tbl[i].attr == EXECUTION_ATTR) {
            printf(ANSI_COLOR_RESET "  %-12s\n", menu_tbl[i].name);
        }
    }

    printf(ANSI_COLOR_RESET "\n");
}

int process_man_cmd(int argc, const char* argv[], menu_command_t* menu_tbl, char echo) {

    if (argv[0] == NULL) {
        printf(ANSI_COLOR_GREEN "'man' command needs an argument !");
        printf(ANSI_COLOR_RESET "\n");
        return ERR_PARAMETER_MISSED;
    }

    for (int i = 0; menu_tbl[i].name; i++) {
        if (!strcmp(argv[0], menu_tbl[i].name)) {
            show_menuitem_in_details(menu_tbl, i);
            return 0;
        }
    }

    if (echo) {
        printf(ANSI_COLOR_GREEN "No manual entry for %s", argv[0]);
        printf(ANSI_COLOR_RESET "\n");
    }
    return ERR_INVALID_PARAMETER;
}

int print_argument_warning_message(int argc, const char* argv[], menu_command_t* menu_tbl, char echo) {
    int i;

    for (i = 0; menu_tbl[i].name; i++)
        if (!strcmp(argv[0], menu_tbl[i].name)) {
            printf(ANSI_COLOR_RED "Invalid Parameter(s)\n");
            printf(ANSI_COLOR_BLUE "SYNOPSIS\n");
            printf(ANSI_COLOR_RESET "%s\n", menu_tbl[i].usage);
            printf(ANSI_COLOR_BLUE "DESCRIPTION\n");
            printf(ANSI_COLOR_RESET "%s\n", menu_tbl[i].desc);
            return i;
        }

    return ERR_INVALID_PARAMETER;
}

int lookup_cmd_tbl(int argc, const char* argv[], menu_command_t* menu_tbl, char echo) {
    const char** pav = NULL;
    int ac, i;

    if (argv[0] == NULL) {
        return ERR_PARAMETER_MISSED;
    }

    pav = argv;
    ac = argc;

    for (i = 0; menu_tbl[i].name; i++) {
        if ((menu_tbl[i].attr == EXECUTION_ATTR) && (!strcmp(argv[0], menu_tbl[i].name))) {
            if (menu_tbl[i].fp != NULL) {
                return menu_tbl[i].fp(ac, pav, menu_tbl);
            }
        }
    }

    for (i = 0; menu_tbl[i].name; i++) {
        if ((menu_tbl[i].attr == DIRECTORY_ATTR) && (!strcmp(argv[0], menu_tbl[i].name))) {
            if (echo) {
                printf(ANSI_COLOR_GREEN "'%s' is a directory !", argv[0]);
                printf(ANSI_COLOR_RESET "\n");
            }
            return ERR_INVALID_PARAMETER;
        }
    }

    if (echo) {
        printf(ANSI_COLOR_RED "'%s' is an unknown parameter !", argv[0]);
        printf(ANSI_COLOR_RESET "\n");
        printf("\nThere are some available parameters:\n");
        show_menu(menu_tbl);
        printf("If you want to know how to use each parameter in detail, type -h at the end.\n\n");
    }
    return ERR_INVALID_PARAMETER;
}
