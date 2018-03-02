//MVY-FS demo
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <fuse.h>

 
#define FSNAME "mvyfs.dat"
#define DATAFILE "data.dat"
#define INDEXFILE "index.dat"
#define NAME 256
#define BLOCKSIZE 4096
#define NUMBER_OF_BLOCKS 5
#define MAX_FILE_SIZE BLOCKSIZE*NUMBER_OF_BLOCKS
#define MAX_CONTENTS 10


// directory data struct
struct directory_record
{ 
	int inode;
	char dir_name[NAME];
 	int number_of_files_contained;
 	int number_of_directories_contained;
 	int files_info[MAX_CONTENTS];
 	int dir_info[MAX_CONTENTS];
};

struct file_record
{
	int inode;
	int file_size;
	char file_name[NAME];
	int data_offsets[NUMBER_OF_BLOCKS];
	int last_seek;
	// Add accessed time
};

void init_fs()
{
	// create mountpoint
}
 
void init_directory()
{

}

void init_file()
{

} 

void create_file()
{

}

void create_dir()
{

}

void write_fs()
{

}

void write_data()
{

}

void write_index()
{

}

void read_fs()
{

}

void read_data()
{

}

void read_index()
{

}

void load_fs()
{

}

void load_data()
{

}

//Driver
int main (void)
{
	umask(0);
	return fuse_main(argc, argv, &mvyfs_opr, NULL); 
}