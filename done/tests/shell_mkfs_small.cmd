mkfs disks/tests.uv6 128 4096
mount disks/tests.uv6
psb

mkdir /small_files
add tests/small_files/10.txt /small_files/10.txt
add tests/small_files/100.txt /small_files/100.txt
add tests/small_files/1000.txt /small_files/1000.txt
add tests/small_files/2048.txt /small_files/2048.txt
add tests/small_files/3096.txt /small_files/3096.txt
add tests/small_files/4096.txt /small_files/4096.txt

lsall
psb
