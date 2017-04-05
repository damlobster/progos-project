/*
 * shell.c
 *
 * Created on: Apr 5, 2017
 * Author: Léonard & Damien
 */
#include <string.h>

#include "direntv6.h"
#include "error.h"
#include "mount.h"

typedef int (*shell_fct)(const char** argc);

struct shell_map {
    const char* name;    // nom de la commande
    shell_fct fct;      // fonction réalisant la commande
    const char* help;   // description de la commande
    size_t argc;        // nombre d'arguments de la commande
    const char* args;   // description des arguments de la commande
};

struct unix_filesystem ufs;
