/*
 * shell.c
 *
 * Created on: Apr 5, 2017
 * Author: Léonard & Damien
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "direntv6.h"
#include "error.h"
#include "mount.h"
#include "direntv6.h"
#include "error.c"
#include "error.h"

#define UNMOUNTED_FS 3;

#define FS_MOUNTED if(u.f == NULL) { \
                        puts("ERROR SHELL: mount the FS before the operation"); \
                        return UNMOUNTED_FS; \
                    }

#define PRINT_ERROR_FS(error) printf("ERROR FS: %s", ERR_MESSAGES[error - ERR_FIRST]);

typedef int (*shell_fct)(void);
typedef int (*shell_fct0)();
typedef int (*shell_fct1)(const char* arg1);
typedef int (*shell_fct2)(const char* arg1, const char* arg2);

struct shell_map {
    const char* name; // nom de la commande
    shell_fct fct; // fonction réalisant la commande
    const char* help; // description de la commande
    size_t argc; // nombre d'arguments de la commande
    const char* args; // description des arguments de la commande
};

struct unix_filesystem u;

int do_help();
int do_exit();
int do_mkfs(const char* arg1);
int do_mkdir(const char** args);
int do_lsall(const char** args);
int do_add(const char** args);
int do_cat(const char** args);

struct shell_map shell_cmds[] =
        {
                { "help", (shell_fct) do_help, "display this help", 0, "" }, //
                { "exit", (shell_fct) do_exit, "exit shell", 0, "" }, //
                { "quit", (shell_fct) do_exit, "exit shell", 0, "" }, //
                { "mkfs", (shell_fct) do_mkfs, "create a new filesystem", 3,
                        "<diskname> <#inodes> <#blocks>" }, //
                { "mkdir", (shell_fct) do_mkdir, "create a new directory", 1,
                        "<dirname>" }, //
                { "lsall", (shell_fct) do_lsall,
                        "list all directories and files contained in the currently mounted filesystem",
                        0, "" }, //
                { "add", (shell_fct) do_add, "add a new file", 2,
                        "<src-fullpath> <dst>" }, //
                { "cat", (shell_fct) do_cat, "display the content of a file", 1,
                        "<pathname>" } //,
//    {"istat", do_istat, "display information about the provided inode", 1, "<inode_nr>"},
//    {"inode", do_inode, "display the inode number of a file", 1, "<pathname>"},
//    {"sha", do_sha, "display the SHA of a file", 1, "<pathname>"},
//    {"psb", do_psb, "Print SuperBlock of the currently mounted filesystem", 0 ""}
        };

int do_help() {
    for (size_t i = 0; i < sizeof(shell_cmds) / sizeof(struct shell_map); i++) {
        printf("- %s %s: %s.", shell_cmds[i].name, shell_cmds[i].args,
                shell_cmds[i].help);
    }

    return 0;
}

int do_exit() {
    exit(0);
}

int do_mkfs(const char* arg1) {
    return -1;
}

int do_mkdir(const char** args) {
    return -1;
}

int do_lsall(const char** args) {
    FS_MOUNTED;

    int error = direntv6_print_tree(&u, 1, "");
    if (error < 0) {
        PRINT_ERROR_FS(error);
    }

    return error;
}

int do_add(const char** args) {
    return -1;
}

int do_cat(const char** args) {
    FS_MOUNTED;

    int inr = direntv6_dirlookup(&u, 1, args[0]);
    if (inr < 0) {
        PRINT_ERROR_FS(inr);
        return inr;
    }

    struct filev6 fv6;
    if (filev6_open(&u, inr, &fv6) < 0) {
        PRINT_ERROR_FS(ERR_IO);
        return ERR_IO;
    }

    unsigned char sector[SECTOR_SIZE + 1];
    sector[SECTOR_SIZE] = '\0';
    while (filev6_readblock(&fv6, sector) > 0) {
        printf("%s", sector);
    }
    putchar('\n');

    return 0;
}

int tokenize_input(char* string, char** args) {
    M_REQUIRE_NON_NULL(string);
    M_REQUIRE_NON_NULL(args);

    int i = 1;
    args[0] = strtok(string, " ");
    while (i <= 3) {
        args[i++] = strtok(NULL, " ");
    }

    return i - 1;
}

struct shell_map* get_command(const char* cmd) {
    for (size_t i = 0; i < sizeof(shell_cmds) / sizeof(struct shell_map); i++) {
        if (strcmp(cmd, shell_cmds[i].name) == 0) {
            return &shell_cmds[i];
        }
    }
    return NULL;
}

int main(void) {
    char* args[4];

    while (!feof(stdin) && !ferror(stdin)) {
        args[0][0] = '\0';
        args[1][0] = '\0';
        args[2][0] = '\0';
        args[3][0] = '\0';

        char line[255] = "";
        //gets(line);
        int n = tokenize_input(line, args);
        if (n != 0) {
            struct shell_map* cmd = get_command(args[0]);
            int result = 0;
            switch (cmd->argc) {
            case 0:
                result = ((shell_fct0) cmd->fct)();
                break;
            case 1:
                result = ((shell_fct1) cmd->fct)(args[1]);
                break;
            case 2:
                result = ((shell_fct2) cmd->fct)(args[1], args[2]);
                break;
            default:
                puts("Wrong number of arguments");
                break;
            }

            printf("result: %d", result);

        }
    }
    return 0;
}
