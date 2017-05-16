test-add:
	cd .. && make clean debug
	rm -f test_add.uv6
	cd .. && ./shell < tests/smb.cmd 2>&1 > tests/test_add.log
	cd .. && make clean all
	cd .. && ./shell < tests/shab.cmd 2>&1 | sed 's/.* //' | tail -n13 > tests/sha_uv6.log
	sha256sum files/*.txt | sed 's/ .*//' | tail -n14 > sha_unix.log
	diff -ry sha_uv6.log sha_unix.log
