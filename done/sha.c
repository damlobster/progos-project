#include <openssl/sha.h>
#include "sha.h"
#include "error.h"
#include "filev6.h"
#include "inode.h"

void print_sha(const unsigned char* sha) {
    if (sha == NULL) {
        puts("SHA is NULL");
    }

    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", sha[i]);
    }
    putchar('\n');
}

void print_sha_inode(const struct unix_filesystem *u, struct inode inode,
        int inr) {
    if (u == NULL || !(inode.i_mode & IALLOC)) {
        return;
    } else {
        if (inode.i_mode & IFDIR) {
            printf("SHA inode %d: no SHA for directories.\n", inr);
            return;
        } else {
            printf("SHA inode %d: ", inr);
        }
    }

    struct filev6 fv6;
    if (filev6_open(u, inr, &fv6) != 0) return; //file open error

    SHA256_CTX sha;
    SHA256_Init(&sha);

    unsigned char sector[SECTOR_SIZE];
    int code;
    while ((code = filev6_readblock(&fv6, sector)) > 0) {
        SHA256_Update(&sha, sector, code);
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha);

    print_sha(hash);
}
