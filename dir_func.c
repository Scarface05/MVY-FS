#include "header.h"

// Initialises a directory structure and returns the created structure to the calling function
struct directory_record init_directory(const char *path)
{
	// Tmp struct for the new node
	struct directory_record new_dir_node;
	strcpy(new_dir_node.dir_name,path);
	new_dir_node.number_of_files_contained = 0;
	new_dir_node.deleted = 0;
	new_dir_node.size = 0;
	memset(new_dir_node.files_info,'\0',MAX_NO_OF_FILES);
	// add directories contained, one for parent and another for itself
	new_dir_node.number_of_directories_contained = 2;	
	return new_dir_node;
}


int create_dir(const char *path)										
{
	// file descriptor for the index.data file
	int index_fd = open("index.dat",O_RDWR);
	// offset specifying the end of index while.
	int end_index_file = lseek(index_fd, 0, SEEK_END);
	// set the file pointer at the beginning  of the index file
	lseek(index_fd, 0, SEEK_SET);
	// Split the absolute path
	split(path,'/');

	char path_split[2][NAME];
	strcpy(path_split[0], tmp_path[0]);
	strcpy(path_split[1], tmp_path[1]);

	// temp struct to write the filename and offset in the mvyfs.dat file on to the index.dat file
	struct entry_index buf_index_file;
	
	// used to read structs in the index.dat file sequentially.
	int start = 0;
	// read the first entry
	// loop to find the parent path(the directory in which the new directory will reside in)
	while(start < end_index_file)	
	{
		start += read(index_fd,&buf_index_file,sizeof(struct entry_index));

		// printf("path_split %s\n\n", path_split[0]);
		// printf("buf_index_file.name %s\n\n", buf_index_file.name);

		// if parent directory found)
		if(strcmp(path_split[0],buf_index_file.name) == 0)
		{
			int mvyfs_fd = open("mvyfs.dat",O_RDWR);
			// go to the offset specified in index.dat file
			int offset_mvyfs_file = lseek(mvyfs_fd,buf_index_file.offset,SEEK_SET);

			struct directory_record buf_mvyfs_file;

			read(mvyfs_fd,&buf_mvyfs_file,sizeof(struct directory_record));

			// to keep a track of the position where the subdirectory was found.
			int sub_dir_counter = 2;

			// if more subdirectories can be created in the original directory.
			if(buf_mvyfs_file.deleted == 1)
			{
				continue;
			}	
			if(buf_mvyfs_file.number_of_directories_contained < MAX_NO_OF_DIREC)
			{
				// check to determine if other subdirectories are present.
				// If present, check if the directory with the same name does not exist.
				if(buf_mvyfs_file.number_of_directories_contained > 2)
				{
					// traverse through the subdirectories.
					// printf("-----------------------\n");
					// printf("%d\n", buf_mvyfs_file.number_of_directories_contained);
					// printf("-----------------------\n");
					
					int pos_to_insert = -1;
					// flag to determine if a position to insert has been found.
					int found_pos_insert = 0;
					
					while(sub_dir_counter < MAX_NO_OF_DIREC)
					{
						if((strcmp(buf_mvyfs_file.dir_info[sub_dir_counter], "\0") == 0) && (found_pos_insert == 0))
						{
							pos_to_insert = sub_dir_counter;
							found_pos_insert = 1;
						}
						// printf("buf_mvyfs_file.dir_info %s\n", buf_mvyfs_file.dir_info[sub_dir_counter]);
						// printf("path_split %s\n", path_split[1]);
						if( strcmp(buf_mvyfs_file.dir_info[sub_dir_counter],path_split[1]) == 0 )
						{
							close(index_fd);
							close(mvyfs_fd);
							printf("Directory Already Exists\n");
							return -1;
						}
						sub_dir_counter++;
					}
					// set the insert position to last empty location
					if(found_pos_insert)
					{
						sub_dir_counter = pos_to_insert;
					}
				}

				printf("Directory Created\n");

				strcpy(buf_mvyfs_file.dir_info[sub_dir_counter],path_split[1]);
				buf_mvyfs_file.number_of_directories_contained++;

				struct directory_record new_dir_node = init_directory(path_split[1]);
				lseek(mvyfs_fd, buf_index_file.offset, SEEK_SET);

				// update the parent directory structure and write it back on to the mvyfs.dat
				write(mvyfs_fd, &buf_mvyfs_file, sizeof(struct directory_record));
				offset_mvyfs_file = lseek(mvyfs_fd,0,SEEK_END);
				// write the new directory on to the filesystem
				write(mvyfs_fd,&new_dir_node,sizeof(struct directory_record));

				struct entry_index dir_entry;
				strcpy(dir_entry.name,path);
				dir_entry.offset = offset_mvyfs_file;
				dir_entry.is_dir = 1;
				lseek(index_fd,0,SEEK_END);
				// write the offset to the index.dat file
				write(index_fd,&dir_entry,sizeof(struct entry_index));
				
				close_files(index_fd, mvyfs_fd, -1);
				return 0;
			}
			else
			{
				close_files(index_fd, mvyfs_fd, -1);
				printf;("Cannot make more than %d number of directories!\n", MAX_NO_OF_DIREC);
				return -1;
			}
		}
	}
	close_files(index_fd, -1, -1);
	printf("The parent directory does not exist!\n");
	return -1;
}

int remove_dir(const char *path, int index_offset)
{

	// file descriptor for the index.data file
	int index_fd = open("index.dat",O_RDWR);
	int mvyfs_fd = open("mvyfs.dat",O_RDWR);
	// Split the absolute path
	split(path,'/');

	char path_split[2][NAME];
	strcpy(path_split[0], tmp_path[0]);
	strcpy(path_split[1], tmp_path[1]);

	// go to the offset specified in index.dat file
	index_offset = find_offset(path, index_offset);
	if(index_offset == -1)
	{
		printf("No such directory found!\n");
		close_files(index_fd, mvyfs_fd ,-1);
		return -1;
	}
	struct entry_index buf_index_file;
	lseek(index_fd, index_offset, SEEK_SET);
	read(index_fd, &buf_index_file, sizeof(struct entry_index));
	
	struct directory_record buf_mvyfs_file;
	lseek(mvyfs_fd, buf_index_file.offset, SEEK_SET);
	read(mvyfs_fd, &buf_mvyfs_file, sizeof(struct directory_record));

	// if a file with the same name has been deleted continue searching.
	if(buf_mvyfs_file.deleted == 1)
	{
		// continue to check if the file exists
		if( remove_dir(path, index_offset + sizeof(struct entry_index)) == -1) 
		{
			close_files(index_fd, mvyfs_fd, -1);
			return -1;
		}
	}		
	// check if the directory is empty
	else if(buf_mvyfs_file.number_of_directories_contained > 2 || buf_mvyfs_file.number_of_files_contained > 0)
	{
		//printf(" no of directoties in %s %d\n", buf_mvyfs_file.dir_name,buf_mvyfs_file.number_of_directories_contained);
		printf("Target Directory is not empty\n");
		close_files(index_fd, mvyfs_fd ,-1);
		return -1;
	}
	// deleting the file
	else
	{
		int index_offset_parent = find_offset(path_split[0], 0);

		struct directory_record parent_buf_mvyfs_file;
		struct entry_index parent_buf_index_file;

		lseek(index_fd, index_offset_parent, SEEK_SET);
		read(index_fd, &parent_buf_index_file,sizeof(struct entry_index));

		lseek(mvyfs_fd, parent_buf_index_file.offset, SEEK_SET);
		read(mvyfs_fd, &parent_buf_mvyfs_file, sizeof(struct directory_record));

		while(parent_buf_mvyfs_file.deleted == 1)
		{
			index_offset_parent = find_offset(path, index_offset + sizeof(struct entry_index));

			lseek(index_fd, index_offset_parent, SEEK_SET);
			read(index_fd, &parent_buf_index_file,sizeof(struct entry_index));

			lseek(mvyfs_fd, parent_buf_index_file.offset, SEEK_SET);
			read(mvyfs_fd, &parent_buf_mvyfs_file, sizeof(struct directory_record));
		}

		buf_mvyfs_file.deleted = 1;

		// remove entry for child directory from parent
		int index = find_index(parent_buf_mvyfs_file, path_split[1], 1);
		//printf("index %d\n", index);
		parent_buf_mvyfs_file.dir_info[index][0] = '\0';
		parent_buf_mvyfs_file.size -= buf_mvyfs_file.size;
		parent_buf_mvyfs_file.number_of_directories_contained--;

		buf_mvyfs_file.size = 0;
		// update the mvyfs.dat file
		lseek(mvyfs_fd, parent_buf_index_file.offset, SEEK_SET);
		write(mvyfs_fd, &parent_buf_mvyfs_file, sizeof(struct directory_record));
		printf("Deleted\n");
	}
	close_files(index_fd, mvyfs_fd ,-1);
	return 0;
}
