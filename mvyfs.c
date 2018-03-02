// Implement the checklist
// Implement persistence through the use of saved txt files.
// Fast Searching to find the directory name (Hash table / B-tree?)

//random access file demo
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
 
#define FNAME "mvyfs.dat"


// directory data struct
struct directory_record
{ 
	int inode;
	char dir_name[256];
 	int number_of_files_contained;
 	int number_of_directories_contained;
 	struct directory_record *dir_info[10];
 	struct file_record *file_info[10];
};

struct file_record
{
	int inode;
	int file_size;
	char file_name[256];
	char data[4096];
	int end_seek;
	// Add accessed time
};
 
void init_directory()
{

}

void init_file()
{

} 


///////////////////////////////////////////////////////
// Random Access File Handlers
//
 
// open or create the file
FILE *FileOpen(char* Filename)
  { FILE* pFile;
	pFile = fopen(Filename,"rb+");
	if (!pFile)
	  pFile = fopen(Filename,"wb+");
	return pFile; }
 
 
// Write a record to the file
int WriteRecord(FILE *File, int RecNum)
  { if( fseek(File, RecNum * sizeof(Record), SEEK_SET) == 0 )
	  if ( fwrite(&Record,sizeof(Record),1,File) )
		return 1;
	return 0; }
 
 
// read a record from the file
int ReadRecord(FILE *File, int RecNum)
  { if( fseek(File, RecNum * sizeof(Record), SEEK_SET) == 0 )
	  if ( fread(&Record,sizeof(Record),1,File) )
		return 1;
	return 0; }
 
 
int AddRecord(FILE *File)
  { fseek(File,0,SEEK_END);
	fwrite(&Record,sizeof(Record),1,File);
	return (ftell(File) / sizeof(Record)) - 1; }
 
 
 
//////////////////////////////////////////////////////////////
// View a Record
//
 
int ViewRecord (FILE *File, int RecNum)
  { if (! ReadRecord(File,RecNum))
	  { printf("Invalid record\n"); 
		return -1; }
	printf("-----\n");
	printf("Record        : %d\n",RecNum);
	printf("Number Value  : %d\n",Record.number);
	printf("Word Value    : %s\n",Record.word);
	printf("-----\n");  
	return RecNum; }
 
 
 
//////////////////////////////////////////////////////////////
// Add a new record
//
 
int AddNewData(FILE *File)
  { memset(&Record,0,sizeof(Record));
	printf("\nEnter a number : ");
	scanf("%d", &Record.number);
	printf("Enter a word : ");
	scanf(" %s",Record.word);
	return AddRecord(File); }
 
 
 
//////////////////////////////////////////////////////////////
// Edit a record
//
 
int EditRecord(FILE *File, int RecNum)
  { if (! ReadRecord(File,RecNum))
	  { printf("Invalid record\n");  
		return -1; }
	printf("\n-----\n");
	printf("Record        : %d\n",RecNum);
	printf("Number Value  : %d\n",Record.number);
	printf("Word Value    : %s\n",Record.word);
	printf("-----\n");  
	 
	do
	  { while(getchar() != '\n');
		printf("Change Values: Number, Word or Save (N, W or S) ? ");
		switch (toupper(getchar()))
		  { case 'N' :
			  printf("\nEnter new number : ");
			  scanf("%d",&Record.number);
			  break;
			case 'W' : 
			  printf("Enter new word : ");
			  scanf(" %15s", Record.word);
			  break;
			case 'S' :
			  if (WriteRecord(File,RecNum))
				printf("\nRecord #%d updated\n",RecNum);
			  return RecNum; } }
	while(1);
	return -1; }
 
 
////////////////////////////////////////////////////////////////
// List records
// 
 
void ListRecords(FILE *File )
  { int i = 0;
	printf("\nRecord     Number\tWord\n\n");
	while (ReadRecord(File,i))
	  { printf("%3d%16d\t%s\n",i,Record.number,Record.word); 
		i++; }
	printf("\n\n"); }
 
 
 
////////////////////////////////////////////////////////
// this is for demonstration purposes only
// you would not do this in a real program
void InitFile(FILE* File)
 { int x, y;
   memset(&Record,sizeof(Record),0);
   for (x = 0; x < 10; x++)
	  { Record.number = rand();
		for (y = 0; y < ((Record.number % 15) + 1); y++)
		  Record.word[y] = (rand() % 26) + 'a';
		Record.word[y] = 0;
		if (! WriteRecord(File,x))
		  printf("Oh drat!");  } }
  
 
 
//////////////////////////////////////////////////////////
// program mains
//
int main (void)
  { int Rec = 0; // record number
	FILE *File;
 
	srand(time(NULL));
 
	File = FileOpen(FNAME); 
	if (!File)
	  { printf("Curses foiled again!\n\n");
		exit(-1); }
 
	printf("Random Access File Demonstration\n\n");
  
	do
	  { printf("Menu : Dummy, Add, Edit, View, List, Quit (D, A, E, V, L or Q) : ");
		switch(toupper(getchar()))
		  { case 'D' :
			  printf("Creating dummy file of 10 entries\n");
			  InitFile(File);
			  break;
			case 'A' :
			  Rec = AddNewData(File);
			  printf("Record #%d Added\n\n", Rec);
			  break;              
			case 'E' :
			  printf("\nRecord number (-1 Cancels): ");
			  scanf("%d",&Rec);
			  if (Rec > -1)
				EditRecord(File,Rec);
			  break;
			case 'V' :
			  printf("\nRecord number (-1 Cancels): ");
			  scanf("%d",&Rec);
			  if (Rec > -1)
				ViewRecord(File,Rec);
			  break;
			case 'L' :
			  ListRecords(File);
			  break;
			case 'Q' :
			  fclose(File);
			  return 0; } 
			   
		 while(getchar() != '\n'); }
	while (1); 
	return 0; }