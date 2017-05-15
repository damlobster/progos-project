mkfs disks/ourcode.uv6 128 4096
mount disks/ourcode.uv6
psb

mkdir /code
add bmblock.c /code/bmblock.c
add error.h /code/error.h
add inode.h /code/inode.h
add sha.c /code/sha.c
add test-dirent.c /code/test-dirent.c
add bmblock.h /code/bmblock.h
add filev6.c /code/filev6.c
add mount.c /code/mount.c
add sha.h /code/sha.h
add test-file.c /code/test-file.c
add direntv6.c /code/direntv6.c
add filev6.h /code/filev6.h
add mount.h /code/mount.h
add shell.c /code/shell.c
add test-ibm-fbm.c /code/test-ibm-fbm.c
add direntv6.h /code/direntv6.h
add fs.c /code/fs.c
add sector.c /code/sector.c
add test-bitmap.c /code/test-bitmap.c
add test-inodes.c /code/test-inodes.c
add error.c /code/error.c
add inode.c /code/inode.c
add sector.h /code/sector.h
add test-core.c /code/test-core.c
add test-inodes-create.c /code/test-inodes-create.c

lsall
psb

