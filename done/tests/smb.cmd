mkfs disks/big.uv6 128 4096
mount disks/big.uv6
psb

add tests/small_files/4200.txt /4200.txt
add tests/small_files/8192.txt /8192.txt
add tests/small_files/917504.txt /917504.txt

lsall
psb
