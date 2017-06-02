/*
 * test-dirent.c
 *
 * Created on: Mar 29, 2017
 * Author: Léonard Berney & Damien Martin
 */
#include "unixv6fs.h"
#include "mount.h"
#include "direntv6.h"

int test(const struct unix_filesystem *u) {
    return direntv6_print_tree(u, 1, "");
}
