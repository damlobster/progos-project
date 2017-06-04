/*
 * test-dirent.c
 *
 * Created on: Mar 29, 2017
 * Author: Léonard Berney & Damien Martin
 */
#include "unixv6fs.h"
#include "mount.h"
#include "direntv6.h"

/**
 * Tests for direntv6.c
 * @param u the file system
 * @return <0 if error occured
 */
int test(const struct unix_filesystem *u) {
    return direntv6_print_tree(u, ROOT_INUMBER, "");
}
