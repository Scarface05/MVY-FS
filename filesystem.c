#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_INODE 1000
#define MAX_DIRECTORY_FILE_SYSTEM 100
#define MAX_FILE_IN_DIR 10

struct inode{
	//ushort  di_mode;     			/* mode and type of file */
	//short   di_nlink;    			/* number of links to file */
	//mode_t	mode;				/* mode of the file */
	char type;						/* type of a file */
	ushort  di_uid;      			/* owner's user id */
	ushort  di_gid;      			/* owner's group id */
	off_t   data_block_offset;     	/* offset number where the file data actually stored */
	time_t  atime;    				/* time last accessed */
	time_t  mtime;    				/* time last modified */
	time_t  ctime;    				/* time created */
};

int inode_count = 0;

int openDiskBlock(char *filename)
{
	int ret = open(filename, O_RDWR);
	return ret;
}
char **split(char *path,char delimiter)
{
	char **array = (char**)malloc(sizeof(char*)*2);
	int i = 0;
	int stop = -1;
	for(i=strlen(path)-1;i>-1;i--)
	{
		if (path[i] == delimiter)
		{
			stop = i;
			break;
		}
	}
	if (stop == -1)
	{
		return NULL;
	}
	else
	{
		char *part1 = (char*)malloc(100);
		char *part2=(char*)malloc(100);
		for(i=0; i<stop;i++)
			part1[i] = path[i];
		part1[i] = '/';
		part1[stop+1] = '\0';
		for (i=stop+1; i<=strlen(path); i++)
			part2[i-(stop+1)] = path[i];
		array[0] = part1;
		array[1] = part2;
		return array;
	}
}
int readdir(int file_des,char **arr)
{
	int i = 0;
	int dirPtr = 0;
	int length_of_dir_search = strlen(arr[0]);
	char buf[strlen(arr[0])+1];
	char tmp_buf = '\0';
	int found = 0;
	int size = lseek(file_des,0,SEEK_END);
	lseek(file_des,0,SEEK_SET);
	while(i<size)
	{
		read(file_des,buf,length_of_dir_search);
		// printf("buf %d\n", buf[13]);
		// printf("arr %d\n", arr[0][13]);
		// printf("strcmp %d\n", strcmp(buf,arr[0]));
		if(strcmp(buf,arr[0]) == 0)
		{
			found = 1;
			printf("inside while\n");
			break;
		}
		else
		{
			lseek(file_des,50-length_of_dir_search,SEEK_CUR);
			i+=size;
		}
	}
	if(found == 1)
	{

		char str[strlen(arr[0])+strlen(arr[1]) + 1];
		strcat(str,arr[0]);
		strcat(str,arr[1]);
		str[strlen(str)] = '\0';
		int length_of_file_search = strlen(str);
		while(tmp_buf != '\n')
		{
			printf("%c\n", tmp_buf);
			read(file_des, &tmp_buf,1);
		}
		printf("outside\n");
		while(tmp_buf != '\n')
		{
			int ptr_file_dir = 0;
			read(file_des,buf,1);		// for tab to read.
			int ptrFile = 0;
			while(tmp_buf != '\n')
			{
				read(file_des,buf,1);
				if(ptr_file_dir == length_of_file_search && !strcmp(buf,"\t"))
					perror("File Already exists");
				else if(ptr_file_dir != length_of_file_search && !strcmp(buf,"\t"))
				{
					// move to next file  i.e, break
					while(strcmp(buf,"\n"))
						read(file_des,buf,1);
					read(file_des,buf,1);
					break;
				}	
				else if(ptr_file_dir != length_of_file_search && !strcmp(buf,"\n"))
				{
					read(file_des,buf,1);
					break;
				}
				else if( (ptr_file_dir != length_of_file_search))
				{
					char *a = &str[ptr_file_dir];
					if(!strcmp(buf, a)){
						ptrFile++;
						continue;
					}
				}

			}
		}
		int current_offset = lseek(file_des,-1,SEEK_CUR);
		if(current_offset % 50 == 0)
			perror("Can not have block size more than 50");
		else
		{
			char *string[5];
			write(file_des,"\t",1);
			write(file_des,str,strlen(str));
			write(file_des,"\t",1);
			current_offset = lseek(file_des,0,SEEK_CUR);
			size = lseek(file_des,1,SEEK_END);
			sprintf(*string,"%d",size+1);
			write(file_des,string,strlen(*string)); 							// need to write length of inode_count in third argument
			lseek(file_des,-(current_offset+1) ,SEEK_END);	// return to same previous point.
			write(file_des,"\n!",2);

			int inode_des = open("/home/shikhar/Desktop/USP/inode.txt",O_RDWR);
			int data_des = open("/home/shikhar/Desktop/USP/data.txt",O_RDWR);
			lseek(file_des,1,SEEK_END);
			write(file_des,str,strlen(str));
			size = lseek(inode_des,0,SEEK_END);
			sprintf(*string,"%d",size+1);
			write(file_des,string,strlen(*string));					// need to write length of inode_count in third argument
			inode_count++;
			srand(time(NULL));
			char *choice[2] = {"1000","2000"};

			write(inode_des,"d\t",2);

			int option_choosen = rand()%2;
			
			write(inode_des,choice[option_choosen],4);
			write(inode_des,"\t",1);

			write(inode_des,choice[option_choosen],4);
			write(inode_des,"\t",1);

			time_t mytime = time(NULL);
    		char * time_str = ctime(&mytime);
    		time_str[strlen(time_str)-1] = '\0';

    		write(inode_des,time_str,strlen(time_str));
			write(inode_des,"\t",1);

			write(inode_des,time_str,strlen(time_str));
			write(inode_des,"\t",1); 

			write(inode_des,time_str,strlen(time_str));
			write(inode_des,"\t",1); 
			   		
		}

	}
	else
	{
		perror("Wrong path specified");
		return -1;
	}
}
int mkdir_mvyfs(char *path)
{
	int dir_des = openDiskBlock("/home/shikhar/Desktop/USP/dirEntry.txt");
	if(inode_count > MAX_INODE)
	{
		perror("No space");
		return 0;
	}
	else
	{
		char **array = split(path,'/');
		readdir(dir_des,array);
		return 0;
	}
}

int main(void)
{
	int filedes = openDiskBlock("/home/shikhar/Desktop/USP/dirEntry.txt");
	char **arr = split("/tmp/example/test",'/');
	readdir(filedes,arr);
}