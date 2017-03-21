
#include "filev6.h"
#include "error.h"
#include "inode.h"
#include "sector.h"

int filev6_open(const struct unix_filesystem *u, uint16_t inr, struct filev6 *fv6) {
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(u);

    fv6->u = u;
    fv6->i_number = inr;
    int error = inode_read(u, inr, &fv6->i_node);
    fv6->offset = 0;

    return error;
}

int filev6_readblock(struct filev6 *fv6, void *buf) {
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(buf);

    if (fv6->offset >= inode_getsize(&fv6->i_node)) {
        return 0;
    }

    int sector = inode_findsector(fv6->u, &fv6->i_node, fv6->offset);
    int error = sector_read(fv6->u->f, sector, buf);

    int last_sector_size = inode_getsize(&fv6->i_node) % SECTOR_SIZE;
    if (last_sector_size == 0) {
        last_sector_size = SECTOR_SIZE;
    }

    int read;
    if (fv6->offset == inode_getsize(&fv6->i_node) - last_sector_size) {
        read = last_sector_size;
    } else {
        read = SECTOR_SIZE;
    }

    fv6->offset += SECTOR_SIZE;

    return error < 0 ? error : read;
}
