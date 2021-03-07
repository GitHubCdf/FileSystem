#define BLOCKSIZE	1024	//block size
#define SIZE		1024000	//virtual memory size
#define END		65535	//flag of file end
#define FREE		0	//flag of free
#define ROOTBLOCKNUM	2	//initial block numbers
#define MAXOPENFILE	10	//maximum number of opening files

#pragma pack(1)
typedef struct FCB{
	char filename[11];	//file name
	unsigned char attribute;	//attribute=0:dirctory,atrtibute=1 file
	unsigned short time;	//create time
	unsigned short date;	//create date
	unsigned short first;	//first block
	unsigned long length;	//file length
	char free;	//free=0 fcb free,free=1 already allocated
	char right;	//user right
	char remian[4];
}fcb;

typedef struct FAT{
	unsigned short id;
}fat;

typedef struct USEROPEN{
	char filename[11];
	unsigned char attribute;
	unsigned short time;
	unsigned short date;
	unsigned short first;
	unsigned long length;
	char free;
	char dir[80];		//file direction 
	int rw;		//read or write point
	char fcbstate;		//
	char topenfile;	//	
	unsigned char right;
}useropen;

typedef struct BLOCK0{
	char information[1021];
	unsigned short root;
	unsigned char *startblock;
}block0;

typedef struct DISK{
	block0 boot;
	fat fat1[BLOCKSIZE];
	fat fat2[BLOCKSIZE];
	char data[SIZE/BLOCKSIZE-5][BLOCKSIZE];
}disk;

unsigned char *myvhard;
useropen openfilelist[MAXOPENFILE];
int curdir;
char currentdir[80];
unsigned char *startp;
disk *mydisk;

void startsys();	//start file system
void my_format();	//initial file system
void my_cd(char *dirname);		//change current file dirctory
void my_mkdir(char *dirname);	//create directory
void my_rmdir(char *dirname);	//delete directory
void my_ls(char *dirname);		//listing the file in the current direcoty or the directory path inputed
int my_create(char *filename);	//according to fileaname create file
void my_rm(char *filename);		//according to filename remove file
int my_open(char *filename);		//open file
void my_close(char fd);			//close file
int my_write(char fd);
int do_write(int fd,char *text, int len, char wstyle);
int my_read(char fd);
int do_read(int fd, unsigned long len, char *text);			//
int my_exitsys();						//exit file system
void fcbtouser(fcb *cfcb,useropen *user,char *filename);			//copy information from fcb to useropen 
fcb *search(char *dirname);					//search file according to dirname
int my_malloc();	
void my_free(fcb *f,fcb *pf);
void listopen();	// list the file was opened
void locate_rw(char fd);
void chmod(char *filename);
void my_rename(char *filename);
void setdate(fcb *f);			// set time and date
void cp(char *agr1,char *agr2);	//my copy
int cpfcb(fcb *agr1, fcb *agr2);	//copy file from source to destination
char *path(char *filepath);	//modify path
