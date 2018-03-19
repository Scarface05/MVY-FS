//MVY-FS demo

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define FSNAME "mvyfs.dat"
#define DATAFILE "data.dat"
#define INDEXFILE "index.dat"
#define NAME 256
#define BLOCKSIZE 1024
#define MAX_NUMBER_OF_BLOCKS 1
#define MAX_FILE_SIZE BLOCKSIZE*MAX_NUMBER_OF_BLOCKS
#define MAX_NO_OF_FILES 10
#define MAX_NO_OF_DIREC 10
#define ROOT_DIR "/root"


// An tmp_path to store the parsed paths
char tmp_path[2][NAME];

// directory record struct
struct directory_record
{ 
	int inode;
	int size;
	char dir_name[NAME];
 	int number_of_files_contained;
 	int number_of_directories_contained;
 	char files_info[MAX_NO_OF_FILES][NAME];
 	char dir_info[MAX_NO_OF_DIREC][NAME];
	// flag to check if the file has been deleted
	int deleted;
};

// file record struct
struct file_record
{
	int inode;
	int file_size;
	char file_name[NAME];
	int data_offsets[MAX_NUMBER_OF_BLOCKS];
	// Store the file pointer so that appending becomes easy.
	int file_last_seek;							// offset to keep track where to write content next time in the file
	// flag to check if the file has been deleted
	int deleted;
	// Add accessed time
};

// entry record struct
struct entry_index
{
	char name[NAME];
	int offset;
	int is_dir;		// if 1 => directory else file
};

// Function Prototype to close all the files.
void close_files(int index_fd, int mvyfs_fd, int data_fd);

// Function Prototype to initialise the file structure.
struct file_record init_file(const char *path);

// Function Prototype to initialise the directory structure.
struct directory_record init_directory(const char *path);

// Function Prototype to initialise the filesystem from disk.
void init_fs();

// Function prototype to split the passed path.
void split(const char *path,char delimiter);

// Function prototype to find the offset of the structure in the filesystem.
int find_offset(const char *path, int start);

// Function prototype to find the index where the child directory is stored.
int find_index(struct directory_record dir, const char *path, int flag);

// Function prototype to create a new directory
int create_dir(const char *path);		

// Function prototype to remove a directory
int remove_dir(const char *path, int index_offset);								

// Function prototype to create a new file
int create_file(const char *path ,int index_offset);

// Function prototype to find minimum between two numbers
int min(int a,int b);

// Function prototype to add data to a file
int add_data(const char *path,int index_offset,char *content_to_be_wriiten);

// Function prototype to append data to a file.
int append_data(const char *path, int index_offset,char *content_to_be_wriiten);


