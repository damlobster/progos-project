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
#include "mount.h"
#include "direntv6.h"
#include "error.h"
#include "inode.h"
#include "sha.h"

#define UNMOUNTED_FS 3;

#define FS_MOUNTED if(u.f == NULL) { \
                        puts("ERROR SHELL: mount the FS before the operation"); \
                        return UNMOUNTED_FS; \
                    }

#define PRINT_ERROR_FS(error) printf("ERROR FS: %s\n", ERR_MESSAGES[error - ERR_FIRST]);

typedef int (*shell_fct)(const char** args);

struct shell_map {
    const char* name; // nom de la commande
    shell_fct fct; // fonction réalisant la commande
    const char* help; // description de la commande
    size_t argc; // nombre d'arguments de la commande
    const char* args; // description des arguments de la commande
};

struct unix_filesystem u;

int do_help(const char** args);
int do_exit(const char** args);
int do_mount(const char** args);
int do_mkfs(const char** args);
int do_mkdir(const char** args);
int do_lsall(const char** args);
int do_add(const char** args);
int do_cat(const char** args);
int do_istat(const char** args);
int do_inode(const char** args);
int do_sha(const char** args);
int do_psb(const char** args);

struct shell_map shell_cmds[] = {
    {"help", do_help, "display this help", 0, ""}, //
    {"exit", do_exit, "exit shell", 0, ""}, //
    {"quit", do_exit, "exit shell", 0, ""}, //
    {"mount", do_mount, "mount a filesystem", 1, "<diskname>"}, //
    {"mkfs", do_mkfs, "create a new filesystem", 3, "<diskname> <#inodes> <#blocks>"}, //
    {"mkdir", do_mkdir, "create a new directory", 1, "<dirname>"}, //
    {"lsall", do_lsall, "list all directories and files contained in the currently mounted filesystem", 0, ""}, //
    {"add", do_add, "add a new file", 2, "<src-fullpath> <dst>"}, //
    {"cat", do_cat, "display the content of a file", 1, "<pathname>"},
    {"istat", do_istat, "display information about the provided inode", 1, "<inode_nr>"},
    {"inode", do_inode, "display the inode number of a file", 1, "<pathname>"},
    {"sha", do_sha, "display the SHA of a file", 1, "<pathname>"},
    {"psb", do_psb, "Print SuperBlock of the currently mounted filesystem", 0, ""}
};

int do_istat(const char** args) {
    int inr = atoi(args[0]);
    if (inr < 0) {
        return ERR_INODE_OUTOF_RANGE;
    }

    struct inode in;
    int error = inode_read(&u, inr, &in);
    if (error < 0) {
        return error;
    }
    inode_print(&in);

    return 0;
}

int do_inode(const char** args) {
    int inr = direntv6_dirlookup(&u, 1, args[0]);
    printf("inode: %d", inr);

    return 0;
}

int do_sha(const char** args) {
    FS_MOUNTED;

    int inr = direntv6_dirlookup(&u, 1, args[0]);

    struct inode in;
    int error = inode_read(&u, inr, &in);
    if (error < 0) {
        return error;
    }
    if (in.i_mode & IFDIR) {
        printf("SHA inode %d: no SHA for directories", inr);
    } else {
        print_sha_inode(&u, in, inr);
    }

    return 0;
}

int do_psb(const char** args) {
    FS_MOUNTED;

    mountv6_print_superblock(&u);

    return 0;
}

int do_help(const char** args) {
    for (size_t i = 0; i < sizeof (shell_cmds) / sizeof (struct shell_map); i++) {
        printf("- %s %s: %s.\n", shell_cmds[i].name, shell_cmds[i].args,
                shell_cmds[i].help);
    }

    return 0;
}

int do_exit(const char** args) {
    exit(0);
}

int do_mount(const char** args) {
    return mountv6(args[0], &u);
}

int do_mkfs(const char** args) {
    return -1;
}

int do_mkdir(const char** args) {
    return -1;
}

int do_lsall(const char** args) {
    FS_MOUNTED;

    return direntv6_print_tree(&u, 1, "");
}

int do_add(const char** args) {
    return -1;
}

int do_cat(const char** args) {
    FS_MOUNTED;

    int inr = direntv6_dirlookup(&u, 1, args[0]);
    if (inr < 0) {
        return inr;
    }

    struct inode in;
    int error = inode_read(&u, inr, &in);
    if (error < 0) {
        return error;
    }
    if (in.i_mode & IFDIR) {
        puts("ERROR SHELL: cat on a directory is not defined"); //TODO
        return 1;

    }

    struct filev6 fv6;
    if (filev6_open(&u, inr, &fv6) < 0) {
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
    char* arg = strtok(string, " ");
    args[0] = arg;
    while (arg != NULL && i <= 3) {
        arg = strtok(NULL, " ");
        args[i++] = arg;
    }

    return i - 1;
}

struct shell_map* get_command(const char* cmd) {
    for (size_t i = 0; i < sizeof (shell_cmds) / sizeof (struct shell_map); i++) {
        if (strcmp(cmd, shell_cmds[i].name) == 0) {
            return &shell_cmds[i];
        }
    }
    return NULL;
}

int main(void) {
    char* args[4];

    while (!feof(stdin) && !ferror(stdin)) {

        char line[255];
        memset(line, 0, 255);
        fgets(line, 255, stdin);
        int n = tokenize_input(line, args);
        if (n != 0 && args[0] != NULL) {
            struct shell_map* cmd = get_command(args[0]);
            int result = 0;
            if (cmd == NULL) {
                puts("Unknow command!");
            } else {
                if (n != cmd->argc) {
                    //wrong number of arguments
                } else {
                    result = cmd->fct(args);
                }

                if (result < 0) PRINT_ERROR_FS(result);
                //if (result > 0) PRINT_ERROR_SHELL(result); TODO
            }

        }
    }
    return 0;
}
