/*
 * shell.c
 *
 * Created on: Apr 5, 2017
 * Author: Léonard & Damien
 */
#include "mount.h"
#include "direntv6.h"
#include "error.c"
#include "error.h"

#define UNMOUNTED_FS 3;

#define FS_MOUNTED if(ufs.f == NULL) { \
                        puts("ERROR SHELL: mount the FS before the operation"); \
                        return UNMOUNTED_FS; \
                    }

#define PRINT_ERROR_FS(error) printf("ERROR FS: %s", ERR_MESSAGES[error - error_codes.ERR_FIRST]);

typedef int (*shell_fct)(const char** args);

struct shell_map {
    const char* name; // nom de la commande
    shell_fct fct; // fonction réalisant la commande
    const char* help; // description de la commande
    size_t argc; // nombre d'arguments de la commande
    const char* args; // description des arguments de la commande
};

struct unix_filesystem ufs;

struct shell_map shell_cmds[] = {
    {"help", do_help, "display this help", 0, ""},
    {"exit", do_exit, "exit shell", 0, ""},
    {"quit", do_exit, "exit shell", 0, ""},
    {"mkfs", do_mkfs, "create a new filesystem", 3, "<diskname> <#inodes> <#blocks>"},
    {"mkdir", do_mkdir, "create a new directory", 1, "<dirname>"},
    {"lsall", do_lsall, "list all directories and files contained in the currently mounted filesystem", 0, ""},
    {"add", do_add, "add a new file", 2, "<src-fullpath> <dst>"},
    {"cat", do_cat, "display the content of a file", 1, "<pathname>"},
    {"istat", do_istat, "display information about the provided inode", 1, "<inode_nr>"},
    {"inode", do_inode, "display the inode number of a file", 1, "<pathname>"},
    {"sha", do_sha, "display the SHA of a file", 1, "<pathname>"},
    {"psb", do_psb, "Print SuperBlock of the currently mounted filesystem", 0 ""}
};

int do_help(const char** args) {
    for (int i = 0; i < sizeof (shell_cmds) / sizeof (shell_map); i++) {
        printf("- %s %s: %s.", shell_cmds[i].name, shell_cmds[i].args, shell_cmds[i].help);
    }

    return 0;
}

int do_exit(const char** args) {
    exit(0);
}

int do_mkfs(const char** args) {
    return -1;
}

int do_mkdir(const char** args) {
    return -1;
}

int do_lsall(const char** args) {
    FS_MOUNTED;

    int error = direntv6_print_tree(&ufs, 1, "");
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

    int inr = direntv6_dirlookup(&ufs, 1, args[0]);
    if (inr < 0) {
        PRINT_ERROR_FS(inr);
        return inr;
    }
    
    struct filev6 fv6;
    if (filev6_open(u, inr, &fv6) < 0) {
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
