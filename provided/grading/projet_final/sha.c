#include <openssl/sha.h>
#include "sha.h"
#include "error.h"
#include "filev6.h"
#include "inode.h"

void print_sha(const unsigned char* sha);

/**
 * @brief print the sha of the content of an inode
 * @param u the filesystem
 * @param inode the inocde of which we want to print the content
 * @param inr the inode number
 */
void print_sha_inode(const struct unix_filesystem *u, struct inode inode, int inr) {
    if (u == NULL || !(inode.i_mode & IALLOC) || inr < 0) {
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
    if (filev6_open(u, (uint16_t) inr, &fv6) != 0) {
        return; //file open error
    }
    SHA256_CTX sha;
    SHA256_Init(&sha);

    //correcteur : je ne suis pas forcément d'accord avec le commentaire du correcteur du projet01/2, bien que c'est un peu plus compliqué à gérer que simplement utiliser SHA256 vous fonction marche et vous avez montré savoir utiliser correctement les manpages donc aucune raison de perdre des points sur cet aspect là. Toutefois je n'ai vraiment pas compris en quoi votre implémentation est avantageuse pour les gros file comme expliqué dans votre README, donc je ne pense pas que cela mérite des points bonus.
    unsigned char sector[SECTOR_SIZE];
    int code;
    while ((code = filev6_readblock(&fv6, sector)) > 0) {
        SHA256_Update(&sha, sector, (size_t) code);
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha);

    print_sha(hash);
}

/**
 * @brief print the sha of the content
 * @param content the content of which we want to print the sha
 * @param length the length of the content
 */
//correcteur : vous n'utilisez nulle part cette fonction, évitez les dead code (-0.5).
void print_sha_from_content(const unsigned char *content, size_t length){
    if(!content || length==0){
        return;
    }
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(content, length, hash);
    print_sha(hash);
}

void print_sha(const unsigned char* sha) {
    if (sha == NULL) {
        puts("SHA is NULL");
    }

    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", sha[i]);
    }
    putchar('\n');
}
