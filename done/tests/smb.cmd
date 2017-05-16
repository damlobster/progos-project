mkfs tests/test_add.uv6 196 8192
mount tests/test_add.uv6
psb

add tests/files/0000010.txt /0000010.txt
add tests/files/0000100.txt /0000100.txt
add tests/files/0001000.txt /0001000.txt
add tests/files/0002048.txt /0002048.txt
add tests/files/0003096.txt /0003096.txt
add tests/files/0004096.txt /0004096.txt
add tests/files/0004200.txt /0004200.txt
add tests/files/0008192.txt /0008192.txt
add tests/files/0009000.txt /0009000.txt
add tests/files/0131072.txt /0131072.txt
add tests/files/0132000.txt /0132000.txt
add tests/files/0655361.txt /0655361.txt
add tests/files/0917504.txt /0917504.txt

lsall
psb
