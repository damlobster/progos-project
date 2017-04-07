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

#define UNMOUNTED_FS 3

#define FS_MOUNTED if(u.f == NULL) { \
                        return UNMOUNTED_FS; \
                    }

#define M_THROW_ERROR(_error_) { \
    if(_error_!=0){ \
        return _error_; \
    } \
}

/**
 * signature of a pointer to a shell function
 * @param args the arguments of the method
 * @return the return code of the called function
 */
typedef int (*shell_fct)(const char** args);

/**
 * Struct containing informations of a shell function
 */
struct shell_map {
    const char* name; // nom de la commande
    shell_fct fct; // fonction réalisant la commande
    const char* help; // description de la commande
    size_t argc; // nombre d'arguments de la commande
    const char* args; // description des arguments de la commande
};

/**
 * The mounted filesystem
 */
struct unix_filesystem u;

/**
 * Shell functions prototypes
 */
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

/**
 * Map of the shell functions
 */
struct shell_map shell_cmds[] =
        {
                { "help", do_help, "display this help", 0, "" }, //
                { "exit", do_exit, "exit shell", 0, "" }, //
                { "quit", do_exit, "exit shell", 0, "" }, //
                { "mount", do_mount, "mount a filesystem", 1, "<diskname>" }, //
                { "mkfs", do_mkfs, "create a new filesystem", 3,
                        "<diskname> <#inodes> <#blocks>" }, //
                { "mkdir", do_mkdir, "create a new directory", 1, "<dirname>" }, //
                { "lsall", do_lsall,
                        "list all directories and files contained in the currently mounted filesystem",
                        0, "" }, //
                { "add", do_add, "add a new file", 2, "<src-fullpath> <dst>" }, //
                { "cat", do_cat, "display the content of a file", 1,
                        "<pathname>" }, { "istat", do_istat,
                        "display information about the provided inode", 1,
                        "<inode_nr>" }, { "inode", do_inode,
                        "display the inode number of a file", 1, "<pathname>" },
                { "sha", do_sha, "display the SHA of a file", 1, "<pathname>" },
                { "psb", do_psb,
                        "Print SuperBlock of the currently mounted filesystem",
                        0, "" } };

enum shell_errors {
    SHELL_ERR_FIRST = 1, //
    SHELL_ERR_UNMOUNTED_FS
};

const char* SHELL_ERROR_MSGS[] = { //
        "mount the FS before the operation" //
        };

void print_error(int err) {
    if (err < 0) {
        printf("ERROR FS: %s\n", ERR_MESSAGES[err - ERR_FIRST]);
    } else if (err > 0) {
        printf("ERROR SHELL: %s\n", SHELL_ERROR_MSGS[err - SHELL_ERR_FIRST]);
    }
}
/**
 * Print the status of an inode
 * @param args[0] the inode number
 * @return <0 on error, 0 o/w
 */
int do_istat(const char** args) {
    int inr = atoi(args[0]);
    if (inr < 0) {
        return ERR_INODE_OUTOF_RANGE;
    }

    struct inode in;
    M_THROW_ERROR(inode_read(&u, (uint16_t) inr, &in));
    inode_print(&in);

    return 0;
}

/**
 * Print the inode number of a file or folder (absolute path)
 * @param args[0] the file/folder full path
 * @return <0 on error, 0 o/w
 */
int do_inode(const char** args) {
    int inr = direntv6_dirlookup(&u, 1, args[0]);
    printf("inode: %d", inr);

    return inr;
}

/**
 * Print the SHA of a file.
 * @param args[0] the full path of the file
 * @return <0 on error, 0 o/w
 */
int do_sha(const char** args) {
    FS_MOUNTED;

    int inr = direntv6_dirlookup(&u, 1, args[0]);

    struct inode in;
    M_THROW_ERROR(inode_read(&u, (uint16_t) inr, &in));
    if (in.i_mode & IFDIR) {
        printf("SHA inode %d: no SHA for directories", inr);
    } else {
        print_sha_inode(&u, in, inr);
    }

    return 0;
}

/**
 * Print the superblock
 * @param args -UNUSED-
 * @return 0
 */
int do_psb(const char** args) {
    FS_MOUNTED;

    mountv6_print_superblock(&u);

    return 0;
}

/**
 * Show the help message
 * @param args -UNUSED-
 * @return 0
 */
int do_help(const char** args) {
    for (size_t i = 0; i < sizeof(shell_cmds) / sizeof(struct shell_map); i++) {
        printf("- %s %s: %s.\n", shell_cmds[i].name, shell_cmds[i].args,
                shell_cmds[i].help);
    }

    return 0;
}

/**
 * Exit the shell (same as CTRL-D)
 * @param args -UNUSED-
 * @return -
 */
int do_exit(const char** args) {
    if (u.f != NULL) {
        umountv6(&u);
    }
    exit(0);
}

/**
 * Mount a file system
 * @param args[0] the (relative) path of the file to mount
 * @return <0 on error, 0 o/w
 */
int do_mount(const char** args) {
    if (u.f != NULL) {
        M_THROW_ERROR(umountv6(&u));
    }
    return mountv6(args[0], &u);
}

/**
 * Unimplemented
 * @param args
 * @return
 */
int do_mkfs(const char** args) {
    return -1;
}

/**
 * Unimplemented
 * @param args
 * @return
 */
int do_mkdir(const char** args) {
    return -1;
}

/**
 * List all files present in the file system
 * @param args -UNUSED-
 * @return 0 on success; <0 on error
 */
int do_lsall(const char** args) {
    FS_MOUNTED;

    return direntv6_print_tree(&u, 1, "");
}

/**
 * Unimplemented
 * @param args
 * @return
 */
int do_add(const char** args) {
    return -1;
}

/**
 * Print the content of a file
 * @param args[0] the file fullpath
 * @return 0 on success; <0 on error
 */
int do_cat(const char** args) {
    FS_MOUNTED;

    int inr = direntv6_dirlookup(&u, 1, args[0]);
    if (inr < 0) {
        return inr;
    }

    struct inode in;
    M_THROW_ERROR(inode_read(&u, inr, &in));
    if (in.i_mode & IFDIR) {
        puts("ERROR SHELL: cat on a directory is not defined"); //TODO
        return 0;

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

/**
 * Extract arguments from a line of the shell
 * @param string the line to parse
 * @param args OUT where to store the arguments
 * @return the number of arguments read
 */
int tokenize_input(char* string, char** args) {
    M_REQUIRE_NON_NULL(string);
    M_REQUIRE_NON_NULL(args);

    int i = 0;
    char* arg = strtok(string, " \n");
    args[i] = arg;
    while (arg != NULL && i <= 3) {
        arg = strtok(NULL, " \n");
        args[++i] = arg;
    }

    return i - 1;
}

/**
 * Get the command to execute from the shell_map
 * @param cmd the command name (args[0])
 * @return the pointer to the command, NULL if no match found
 */
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

        char line[255];
        memset(line, 0, 255);
        fgets(line, 255, stdin);
        int n = tokenize_input(line, args);
        if (n != -1 && args[0] != NULL) {
            struct shell_map* cmd = get_command(args[0]);
            int result = 0;
            if (cmd == NULL) {
                puts(
                        "Command not found! Type 'help' to display available commands.");
            } else {
                if ((size_t) n != cmd->argc) {

                } else {
                    result = cmd->fct(&args[1]);
                }

                if (result != 0) print_error(result);
            }

        }
    }
    return 0;
}
