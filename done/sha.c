#include <openssl/sha.h>
#include "sha.h"
#include "error.h"
#include "filev6.h"
#include "inode.h"

void compute_sha_from_content(const unsigned char* content, size_t n, unsigned char* hash) {
    SHA256_CTX sha;
    SHA256_Init(&sha);
    SHA256_Update(&sha, content, n);
    SHA256_Final(hash, &sha);
}

void print_sha(const unsigned char* sha) {
    if (sha == NULL) {
        puts("SHA is NULL");
    }

    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", sha[i]);
    }
}

void print_sha_from_content(const unsigned char *content, size_t length) {
     unsigned char hash[SHA256_DIGEST_LENGTH];
    compute_sha_from_content(content, length, hash);

    print_sha(hash);
}

void print_sha_inode(const struct unix_filesystem *u, struct inode inode, int inr) {
    if (!(inode.i_mode & IALLOC)) {
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
    filev6_open(u, inr, &fv6);

    int code;
    int i = 0;
    unsigned char content[inode_getsize(&inode)];
    while ((code = filev6_readblock(&fv6, content + i * SECTOR_SIZE)) > 0) {
        i++;
    }

    print_sha_from_content(content, inode_getsize(&inode));
    putchar('\n');
}
