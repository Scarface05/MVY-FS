#include "header.h"

int main (int argc, char *argv[])
{

	init_fs();
	create_dir("/root/dir1");
	create_dir("/root/dir1/dir2");
	create_dir("/root/dir1/dir3");
	remove_dir("/root/dir1/dir2",0);
	remove_dir("/root/dir1",0);
	create_file("/root/dir1/file1",0);
	create_file("/root/dir1/file1",0);
	add_data("/root/dir1/file1", 0, "Hello");
	append_data("/root/dir1/file1", 0, "Hello");
	return 0;
}