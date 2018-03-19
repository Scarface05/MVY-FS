#include "header.h"

void close_files(int index_fd,int mvyfs_fd,int data_fd)
{
	if(index_fd != -1)
		close(index_fd);
	if(mvyfs_fd != -1)
		close(mvyfs_fd);
	if(data_fd != -1)
		close(data_fd);
}


void init_fs()
{
	int index_fd = open("index.dat",O_RDWR | O_CREAT | O_EXCL, 0666);
	if(index_fd == -1)
	{
		close(index_fd);
		return ;
	}
	int mvyfs_fd = open("mvyfs.dat",O_RDWR | O_CREAT | O_EXCL, 0666);
	int data_fd = open("data.dat",O_RDWR | O_CREAT | O_EXCL, 0666);
	
	struct entry_index initial_entry_index;
	strcpy(initial_entry_index.name, ROOT_DIR);

	initial_entry_index.offset = lseek(mvyfs_fd,0,SEEK_CUR);
	write(index_fd, &initial_entry_index, sizeof(struct entry_index));

	
	struct directory_record initial_entry_mvyfs;
	initial_entry_mvyfs = init_directory(ROOT_DIR);
	write(mvyfs_fd, &initial_entry_mvyfs, sizeof(struct directory_record));
	printf("Mountpoint Created\n");

	close_files(index_fd,mvyfs_fd,data_fd);
}


// Parses the path to split it into 2 parts, the parent directory and the child
void split(const char *path,char delimiter)
{
	// Contains the parent directory
	tmp_path[0][0] = '\0';
	// Contains the child
	tmp_path[1][0] = '\0';
	int i = 0;
	// Denotes the point at which split will occur.
	int stop = -1;
	// Finding the point for split.
	for(i = strlen(path) - 1; i > -1; i--)
	{
		if (path[i] == delimiter)
		{
			stop = i;
			break;
		}
	}
	// If invalid path is specified, return
	if (stop == -1)
	{
		printf("Invalid Path Name Specified\n");
	}
	// else split and save the path into tmp path buffers
	else
	{
		char part1[NAME];
		char part2[NAME];
		for(i = 0; i < stop; i++)
		{
			part1[i] = path[i];
		}
		part1[stop] = '\0';
		for(i = stop + 1; i <= strlen(path); i++)
		{
			part2[i-(stop+1)] = path[i];
		}
		part2[i-1] = '\0';
		strcpy(tmp_path[0],part1);
		strcpy(tmp_path[1],part2);
	}
	// printf("---------------------------------------\n");
	// printf("In split\n");
	// printf("tmp_path[0] %s\n", tmp_path[0]);
	// printf("tmp_path[1] %s\n", tmp_path[1]);
	// printf("---------------------------------------\n");
}

int find_offset(const char *path, int start)
{
	// file descriptor for the index.data file
	int index_fd = open("index.dat",O_RDWR);
	// offset specifying the end of index while.
	int end_index_file = lseek(index_fd, 0, SEEK_END);
	// set the file pointer at the beginning  of the index file
	lseek(index_fd, start, SEEK_SET);

	struct entry_index buf_index_file;
	// loop to find the parent path(the directory in which the new directory will reside in)
	while(start < end_index_file)	
	{
		start += read(index_fd,&buf_index_file,sizeof(struct entry_index));
		// if parent directory found)
		if(strcmp(path, buf_index_file.name) == 0)
		{
			close_files(index_fd, -1, -1);
			return start - sizeof(struct entry_index);
		}
	}
	close_files(index_fd, -1, -1);
	return -1;
}

int find_index(struct directory_record dir, const char *path, int flag)
{	
	int i = 0;
	// for directory
	if(flag == 1)
	{
		for(i = 0; i < dir.number_of_directories_contained; i++)
		{
			if(strcmp(path, dir.dir_info[i]) == 0)
			{
				return i;
			}
		}
		return -1;	
	}
	// for file
	else if(flag == 2)
	{
		for(i = 0; i <= dir.number_of_files_contained; i++)
		{
			if(strcmp(path, dir.files_info[i]) == 0)
			{
				return i;
			}
		}
		return -1;	
	}
}


int min(int a,int b)
{
	if(a < b)
		return a;
	else
		return b;
}
