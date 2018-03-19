#include "header.h"

struct file_record init_file(const char *path)
{
	struct file_record new_file_node;
	strcpy(new_file_node.file_name , path);
	new_file_node.deleted = 0;
	new_file_node.file_size = 0;
	new_file_node.file_last_seek = -1;
	memset(new_file_node.data_offsets,-1,MAX_NUMBER_OF_BLOCKS);
	return new_file_node;
}

int create_file(const char *path ,int index_offset)
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
	index_offset = find_offset(path_split[0], 0);
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
		if( create_file(path, index_offset + sizeof(struct entry_index)) == -1) 
		{
			close_files(index_fd, mvyfs_fd, -1);
			return -1;
		}
	}
	else if(buf_mvyfs_file.number_of_files_contained < MAX_NO_OF_FILES)
	{
		// index where this file is located
		int index = find_index(buf_mvyfs_file,path_split[1],2);
		// if file doesn't already exist
		if(index == -1)
		{
			// writing file record 
			struct file_record new_file_node = init_file(path_split[1]);
			int file_record_offset = lseek(mvyfs_fd,0,SEEK_END);
			write(mvyfs_fd,&new_file_node,sizeof(struct file_record));

			// updating parent directory record for the file
			// printf("----------------------------------------\n");
			// printf("%s\n", path_split[1]);
			// printf("----------------------------------------\n");

			lseek(mvyfs_fd,buf_index_file.offset,SEEK_SET);
			read(mvyfs_fd,&buf_mvyfs_file,sizeof(struct directory_record));
			//printf("%d\n", buf_mvyfs_file.number_of_files_contained);


			strcpy(buf_mvyfs_file.files_info[buf_mvyfs_file.number_of_files_contained] , path_split[1]);
			buf_mvyfs_file.number_of_files_contained++;
			lseek(mvyfs_fd,buf_index_file.offset,SEEK_SET);
			write(mvyfs_fd,&buf_mvyfs_file,sizeof(struct directory_record));

			lseek(mvyfs_fd,buf_index_file.offset,SEEK_SET);
			read(mvyfs_fd,&buf_mvyfs_file,sizeof(struct directory_record));
			//printf("%s\n", buf_mvyfs_file.files_info[0]);

			// adding entry in index table
			struct entry_index new_entry;
			strcpy(new_entry.name, path);
			new_entry.offset = file_record_offset;
			new_entry.is_dir = 0;
			lseek(index_fd,0,SEEK_END);
			write(index_fd, &new_entry,sizeof(struct entry_index));

			close_files(index_fd, mvyfs_fd, -1);
			printf("Created file\n");
			return 0;
		}
		else
		{
			printf("File already exists!\n");
			close_files(index_fd, mvyfs_fd, -1);
			return -1;
		}
	}
	else
	{
		printf("Cannot create more than %d files per directory\n",MAX_NO_OF_FILES);
		close_files(index_fd, mvyfs_fd, -1);
		return -1;
	}
}

int add_data(const char *path,int index_offset,char *content_to_be_wriiten)
{
	int index_fd = open("index.dat",O_RDWR);
	int mvyfs_fd = open("mvyfs.dat",O_RDWR);
	split(path,'/');
	char path_split[2][NAME];
	strcpy(path_split[0], tmp_path[0]);
	strcpy(path_split[1], tmp_path[1]);

	// go to the offset specified in index.dat file
	index_offset = find_offset(path, index_offset);
	if(index_offset == -1)
	{
		printf("parent directory doesn't exist file found!\n");
		close_files(index_fd, mvyfs_fd, -1);
		return -1;
	}

	struct entry_index buf_index_file;
	lseek(index_fd, index_offset, SEEK_SET);
	read(index_fd, &buf_index_file, sizeof(struct entry_index));

	struct file_record buf_mvyfs_file;
	lseek(mvyfs_fd, buf_index_file.offset, SEEK_SET);
	read(mvyfs_fd, &buf_mvyfs_file, sizeof(struct file_record));

	// if a file with the same name has been deleted continue searching.
	if(buf_mvyfs_file.deleted == 1)
	{
		// continue to check if the file exists
		if( add_data(path, index_offset + sizeof(struct entry_index),content_to_be_wriiten) == -1)
		{
			close_files(index_fd, mvyfs_fd, -1);
			return -1;
		}
	}
	else
	{
		if(buf_mvyfs_file.file_size >= BLOCKSIZE*MAX_NUMBER_OF_BLOCKS)
		{
			printf("file size cannot be more than %dkb\n",buf_mvyfs_file.file_size/1024);
			close_files(index_fd, mvyfs_fd, -1);
			return -1;
		}
		else
		{
			// if first time the content is written.
			int len_content = strlen(content_to_be_wriiten);
			int no_of_block_required;
			int data_fd = open("data.dat",O_RDWR);
			int data_offsets_file;

			if(buf_mvyfs_file.file_last_seek == -1)
			{
				// to know how many blocks does the content which is need to be wriiten requires
				//no_of_block_required = len_content%BLOCKSIZE;

				// content doesn't exceed 1 block
				// if content is spans more than one block.
				no_of_block_required = ceil(len_content/BLOCKSIZE);
				if(no_of_block_required >= MAX_NUMBER_OF_BLOCKS)
					no_of_block_required = MAX_NUMBER_OF_BLOCKS;
				data_offsets_file = lseek(data_fd,0,SEEK_END);


				// finding parent directory to increament the size of it.
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
				// updating the content of the file_record.
				if(no_of_block_required == MAX_NUMBER_OF_BLOCKS)
				{
					write(data_fd, &content_to_be_wriiten,BLOCKSIZE*MAX_NUMBER_OF_BLOCKS);
					buf_mvyfs_file.file_last_seek = data_offsets_file + BLOCKSIZE*MAX_NUMBER_OF_BLOCKS;
				
					parent_buf_mvyfs_file.size += BLOCKSIZE*MAX_NUMBER_OF_BLOCKS;

					printf("File size greater than %dbytes,written only %dbytes of file content\n",BLOCKSIZE*MAX_NUMBER_OF_BLOCKS,BLOCKSIZE*MAX_NUMBER_OF_BLOCKS);
				}
				else
				{
					write(data_fd,&content_to_be_wriiten,len_content);
					buf_mvyfs_file.file_last_seek = data_offsets_file + len_content;

					parent_buf_mvyfs_file.size += len_content;
				}
				int i = 0;
				int add = 0;
				// updating data_offset 
				while(i<no_of_block_required)
				{
					buf_mvyfs_file.data_offsets[i] = data_offsets_file + add;
					add += BLOCKSIZE;
					i++;
				}

				lseek(mvyfs_fd, index_offset_parent, SEEK_SET);
				write(mvyfs_fd, &parent_buf_mvyfs_file, sizeof(struct directory_record));

				// updating file_node in mvyfs.dat file
				lseek(mvyfs_fd, buf_index_file.offset, SEEK_SET);
				write(mvyfs_fd,&buf_mvyfs_file,sizeof(struct file_record));

				close_files(index_fd, mvyfs_fd, data_fd);
				printf("Successfully written by add data\n");
				return 0;

			}
			// if there is content is already present in the file, then overwrting the content of file
			else
			{
				printf("File Already has a content if u want to append call append function\n");
				close_files(index_fd, mvyfs_fd, data_fd);
				return -1;
			}
		}
	}
}
int append_data(const char *path, int index_offset,char *content_to_be_wriiten)
{
	int index_fd = open("index.dat",O_RDWR);
	int mvyfs_fd = open("mvyfs.dat",O_RDWR);
	split(path,'/');
	char path_split[2][NAME];
	strcpy(path_split[0], tmp_path[0]);
	strcpy(path_split[1], tmp_path[1]);

	// go to the offset specified in index.dat file
	index_offset = find_offset(path, index_offset);
	if(index_offset == -1)
	{
		printf("parent directory doesn't exist file found!\n");
		close_files(index_fd, mvyfs_fd, -1);
		return -1;
	}

	struct entry_index buf_index_file;
	lseek(index_fd, index_offset, SEEK_SET);
	read(index_fd, &buf_index_file, sizeof(struct entry_index));

	struct file_record buf_mvyfs_file;
	lseek(mvyfs_fd, buf_index_file.offset, SEEK_SET);
	read(mvyfs_fd, &buf_mvyfs_file, sizeof(struct file_record));

	// if a file with the same name has been deleted continue searching.
	if(buf_mvyfs_file.deleted == 1)
	{
		// continue to check if the file exists
		if( append_data(path, index_offset + sizeof(struct entry_index), content_to_be_wriiten) == -1)
		{
			close_files(index_fd, mvyfs_fd, -1);
			return -1;
		}
	}
	else
	{
		int len_content = strlen(content_to_be_wriiten);
		int no_of_block_required;
		int data_fd = open("data.dat",O_RDWR);
		int data_offsets_file;
		// getting the current block in which the last_seek_file is pointing to.
		int current_block_index = buf_mvyfs_file.file_size / BLOCKSIZE;
		if(current_block_index == MAX_NUMBER_OF_BLOCKS -1 && buf_mvyfs_file.file_last_seek == (buf_mvyfs_file.data_offsets[current_block_index] + BLOCKSIZE) )
		{
			printf("max file size exceeded cannot add more data.\n");
			close_files(index_fd, mvyfs_fd, data_fd);
			return -1;
		}

		// finding parent directory to increament the size of it.
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

		// calulating the space available in the current block in order to write some of the content in that block
		int space_available_current_block = BLOCKSIZE - (buf_mvyfs_file.file_last_seek - buf_mvyfs_file.data_offsets[current_block_index]);

		lseek(data_fd, buf_mvyfs_file.file_last_seek , SEEK_SET);

		// getting minimum of the two (content length or the space in the block)
		int minimum_len = min(len_content, space_available_current_block);
		write(data_fd, &content_to_be_wriiten, minimum_len);

		lseek(mvyfs_fd, index_offset_parent, SEEK_SET);
		read(mvyfs_fd, &parent_buf_mvyfs_file,sizeof(struct directory_record));
		parent_buf_mvyfs_file.size += minimum_len;

		buf_mvyfs_file.file_last_seek += minimum_len;

		int difference_in_len = len_content - minimum_len;
		if( difference_in_len > 0)
		{

			char content[difference_in_len];
			strncpy(content,&content_to_be_wriiten[minimum_len],difference_in_len);

			no_of_block_required = ceil(difference_in_len/BLOCKSIZE);
			if(no_of_block_required >= MAX_NUMBER_OF_BLOCKS)
				no_of_block_required = MAX_NUMBER_OF_BLOCKS - (current_block_index + 1);
			data_offsets_file = lseek(data_fd,0,SEEK_END);

			// updating the content of the file_record.
			if(no_of_block_required == 0)
			{
				printf("Cannot allocate more than %d blocks to a file.\n", MAX_NUMBER_OF_BLOCKS);
				close_files(index_fd, mvyfs_fd, data_fd);
				return -1;
			}
			if( no_of_block_required == MAX_NUMBER_OF_BLOCKS - (current_block_index + 1))
			{
				write(data_fd, &content, BLOCKSIZE*(MAX_NUMBER_OF_BLOCKS - (current_block_index + 1)));
				buf_mvyfs_file.file_last_seek = data_offsets_file + BLOCKSIZE*(MAX_NUMBER_OF_BLOCKS - (current_block_index + 1));
				printf("File size greater than %dbytes,written only %dbytes of file content\n",MAX_FILE_SIZE,MAX_FILE_SIZE);
				parent_buf_mvyfs_file.size += BLOCKSIZE*(MAX_NUMBER_OF_BLOCKS - (current_block_index + 1));
			}
			else
			{
				write(data_fd,&content,strlen(content));
				buf_mvyfs_file.file_last_seek = data_offsets_file + strlen(content);
				parent_buf_mvyfs_file.size += strlen(content);
			}
			int i = current_block_index;
			int add = 0;
			// updating data_offset to add the blocks which got added
			while(i<no_of_block_required)
			{
				buf_mvyfs_file.data_offsets[i] = data_offsets_file + add;
				add += BLOCKSIZE;
				i++;
			}

		}
		// updating file_node in mvyfs.dat file
		lseek(mvyfs_fd, index_offset_parent, SEEK_SET);
		write(mvyfs_fd, &parent_buf_mvyfs_file, sizeof(struct directory_record));

		lseek(mvyfs_fd, buf_index_file.offset, SEEK_SET);
		write(mvyfs_fd,&buf_mvyfs_file,sizeof(struct file_record));

		close_files(index_fd, mvyfs_fd, data_fd);
		printf("Successfully written by append\n");
		return 0;

	}
}
