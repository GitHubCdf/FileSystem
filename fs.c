#include"fs.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>

int main()
{
	char str[80];
	char name[70];
	char cmd[10];
	int offset=0;
	int flag=0;		//command  flag
	int count=0;
	int space=0;
	int seek;
	startsys();		//starting file system
	while(1)
	{
		//printf("userfile>>%s\n",(openfilelist+curdir)->filename);
		//printf("userfile first>>%d\n",(openfilelist+curdir)->first);
		//printf("userfile path>>%s\n",(openfilelist+curdir)->dir);
		//printf("./ name %s\n",((fcb*)mydisk->data[(openfilelist)->first])->filename);
		//printf("./ first %d\n",((fcb*)mydisk->data[(openfilelist)->first])->first);
		//printf("./ len %ld\n",((fcb*)mydisk->data[(openfilelist)->first])->length);
		listopen();
		printf("%s>>",currentdir);
		fflush(stdin);		//clear cache
		fgets(str,80,stdin);	
		str[strlen(str)-1]='\0';
		while(*(str+offset) != '\0')	
		{
			if(*(str+offset) != ' ')
				count++;
			else
			{
				if(count != 0 && flag == 0)
				{
					memcpy(cmd,str+offset-count,count);		//copy command
					//printf("cmd%d\n",count);
					cmd[count]='\0';
					count = 0;
					flag = 1;		
				}
				if(count != 0)	//recode the end number of space
				{
					space++;
				}
				else
					space=0;
			}
			offset++;
		}
		//printf("count :%d\n",count);
		if(flag == 1)
		{
			memcpy(name,str+offset-count-space,count);
			name[count]='\0';
			if(strcmp(name,"/\0")!=0)
			{
				offset = strlen(name);
				if(*(name+offset-1)=='/')	//if end of '/' remove the '/' 
					name[offset-1]='\0';
			}
			//printf("name%d\n",count);
			flag = 0;
			count = 0;
			offset = 0;
			space = 0;
		}
		else
		{
			strcpy(cmd,str);	//copy file name
			strcpy(name,"\0");
			count = 0;
			offset = 0;
		}
		//printf("%s,%d,%d\n",cmd,strlen(cmd),space);
		//printf("%s,%d\n",name,strlen(name));
		if(strcmp(cmd,"ls")==0)
		{
			my_ls(name);
		}
		else if(strcmp(cmd,"cd")==0)
		{
			my_cd(name);
		}
		else if(strcmp(cmd,"mkdir")==0)
		{
			my_mkdir(name);
		}
		else if(strcmp(cmd,"rmdir")==0)
		{
			my_rmdir(name);
		}
		else if(strcmp(cmd,"create")==0)
		{
			my_create(name);
		}
		else if(strcmp(cmd,"rm")==0)
		{
			my_rm(name);
		}
		else if(strcmp(cmd,"open")==0)
		{
			curdir = my_open(name);
		}
		else if(strcmp(cmd,"close")==0)
		{
			if(strlen(name)>1)
				printf("Invalid argument\n");
			else
				my_close(*name);
		}
		else if(strcmp(cmd,"write")==0)
		{
			if(strlen(name)>1)
				printf("Invalid argument\n");
			else
				my_write(*name);
		}
		else if(strcmp(cmd,"read")==0)
		{
			if(strlen(name)>1)
				printf("Invalid argument\n");
			else
				my_read(*name);
		}
		else if(strcmp(cmd,"exitsys")==0)
		{
			if(my_exitsys()!=-1)
				break;
		}
		else if(strcmp(cmd,"lseek")==0)
		{
			if(strlen(name)>1)
				printf("Invalid argument\n");
			else{
				printf("input the rw_point offset:");
				scanf("%d",&seek);
				while(( getchar())!='\n');	//clear buffer
				locate_rw(*name);
			}
		}
		else if(strcmp(cmd,"chmod")==0)
		{
			chmod(name);
		}
		else if(strcmp(cmd,"rename")==0)
		{
			my_rename(name);
		}
		else 
		{
			printf("%s:command not found\n",cmd);
		}
	}
	return 0;
}

void startsys()
{
	int offset;
	myvhard = (char*)malloc(SIZE);
	mydisk = (disk*)myvhard;
	FILE *f = fopen("my_fs","r");
	if(f != NULL)
	{
		printf("********************loading filesystem************************\n\n");
		fread(myvhard,sizeof(char),SIZE,f);
		fclose(f);
	}
	else
	{
		printf("********************initial file system***********************\n\n");
		my_format();
	}
	memcpy(openfilelist[0].filename,"/\0",sizeof("/\0"));
	openfilelist[0].attribute = 0;							//
	openfilelist[0].first = ((fcb*)mydisk->data[mydisk->boot.root])->first;		//	format
	memcpy(openfilelist[0].dir,"/\0",sizeof("/\0"));					//	user
	openfilelist[0].fcbstate = 0;								//	open
	openfilelist[0].topenfile = 1;							//	file
	openfilelist[0].free = ((fcb*)mydisk->data[mydisk->boot.root])->free;		//	list
	openfilelist[0].length = ((fcb*)mydisk->data[mydisk->boot.root])->length;		//	
	memcpy(currentdir,"/\0",sizeof("/\0"));
	curdir=0;
	startp = *(mydisk->data);
	for(offset=1;offset<MAXOPENFILE;offset++)
	{
		openfilelist[offset].topenfile=0;
		openfilelist[offset].fcbstate=0;
	}
}

void my_format()
{
	memset(myvhard,0,SIZE);
	fcb *root;
	root = (fcb*)mydisk->data;
	setdate(root);
	setdate(root+1);
	memcpy(root[0].filename,".\0",sizeof(".\0"));
	memcpy(root[1].filename,"..\0",sizeof("..\0"));
	root[0].attribute = root[1].attribute = 0;
	root[0].first = root[1].first = 0;
	root[0].length = root[1].length = 2*sizeof(fcb);
	root[0].free = root[1].free = 1;
	mydisk->fat1[0].id = 1;
	mydisk->fat1[1].id = END;
	mydisk->boot.root = 0;
	mydisk->boot.startblock = *(mydisk->data);
	sprintf(mydisk->boot.information,"size of block:1024; number of blocks:1000");
}

fcb *search(char *dirname)		//search file in directory
{
	int offset = 0;
	int count = 0;
	int i,j;
	int error = 0;
	char subdir[12];
	unsigned short fp;		//file start block
	fcb *f;
	fp = openfilelist[0].first;	//current file start block number
	//printf("search fp1:%d\n",fp);		
	while(*(dirname+offset) != '\0')
	{
		if(*(dirname+offset) == '/')
		{
			memcpy(subdir,dirname+offset-count,count);	//get the subdir
			subdir[count] = '\0';
			count=0;
			if(strlen(subdir)==0)	//root directory
			{
				fp = ((fcb*)mydisk->data[mydisk->boot.root])->first;		//root directory start block number
				memcpy(subdir,".\0",sizeof(".\0"));
			}
			f = (fcb*)mydisk->data[fp];	//find the file start block;
			//printf("start of filename:%s\n",f->filename);
			if(f->attribute==0)	//directory
			{
				if(f->free==1)	//not free
				{
					if(f->length < BLOCKSIZE)	//only need searching in first block
					{
						for(i=0; i< (int)(f->length/sizeof(fcb));i++)	//fcb number
						{
							//printf("subdir:%s,filename:%s\n",subdir,(f+i)->filename);
							if(strcmp(subdir,(f+i)->filename)==0) //found fcb
							{
								fp = (f+i)->first;
								error = 1;
								break;
							}
							error = 0;
						}
					}
					else	//searching in many blocks
					{
						for(i=0;i<(int)(f->length/BLOCKSIZE);i++)
						{
							for(j=0; j < (BLOCKSIZE/sizeof(fcb));j++)
							{
								if(strcmp(subdir,(f+j)->filename)==0)
								{
									fp = (f+j)->first;
									error = 1;
									break;
								}
								error = 0;
							}	
						}
						if(error)
							break;
						else
						{
							fp = mydisk->fat1[fp].id;
							f = (fcb*)mydisk->data[fp];
							for(i=0; i< (int)((f->length%BLOCKSIZE)/sizeof(fcb));i++)
							{
								if(strcmp(subdir,(f+i)->filename)==0)
								{
									fp = (f+i)->first;
									error = 1;
									break;
								}
								error = 0;
							}
						}
					}
					
				}
				else
				{
					error = 0;
					printf("%s:no such file or directory1\n",dirname);
				}
			}
			else
				printf("current file is not directory,please close file\n");
		}
		else
		{
			error = 1;
			count++;
		}
		if(!error)
		{
			printf("%s:no such file or directory2\n",dirname);
			return NULL;
		}
		offset++;
	}
	if(*(dirname+offset-1)=='/')
	{
//		dirname[offset-1]='\0';
		return (fcb*)mydisk->data[fp];
	}
	strcpy(subdir,dirname+offset-count);
	if(strlen(subdir)==0)
	{
		fp = mydisk->boot.root;
		memcpy(subdir,".\0",sizeof(".\0"));
	}
	//printf("search fp :%d",fp);
	f = (fcb*)mydisk->data[fp];
	if((f->attribute) == 0)
	{
		if(f->free == 1)
		{
			for(i=0;i<(f->length/sizeof(fcb));i++)
			{
				if(strcmp(subdir,(f+i)->filename)==0)
				{
					//printf("%s\n",(f+i)->filename);
					//printf("search dir %s\n",subdir);
					return (f+i);
				}
			}
			printf("%s:no such file or directory3\n",dirname);
			return NULL;
		}
		else
		{
			printf("directory is free\n");
			printf("%s:no such file or directory4\n",dirname);
			return NULL;
		}
	}
	else 
	{
		printf("current file is not directory,please close file\n");
		return NULL;
	}
}

void fcbtouser(fcb *cfcb,useropen * user,char *filepath)
{
	
	strcpy(user->filename,cfcb->filename);	//copy fcb information to useropen
	user->first = cfcb->first;
	user->attribute = cfcb->attribute;
	user->time = cfcb->time;
	user->date = cfcb->date;
	user->free = cfcb->free;
	user->right = cfcb->right;
	user->length = cfcb->length;
	strcpy(user->dir,filepath);
	user->rw = 0;
	user->fcbstate = 0;
	user->topenfile = 1;
}

void my_cd(char *dirname)
{
	int offset = 0;
	int count = 0;
	int offset2=0;
	int count2=0;
	char tempdir[80];
	fcb *cfcb;		//currentfcb
	if(openfilelist[0].attribute==1)
		printf("currrent file is not directory,please colse this file\n");
	else 
	{
		cfcb = search(dirname);
		if(cfcb != NULL)
		{
			if(cfcb->attribute==0)		//is directory
			{
				if(strlen(dirname)!=0)	//path is not "\0" 
				{
					if(dirname[0]=='.')	// file path "./....."
					{
						if(dirname[1]=='.')  // file path "../....."
						{
							while(*(currentdir+offset) != '\0')
							{
								if(*(currentdir+offset)=='/')
									count = 0;
								else 
									count++;
								offset++;
							}
							currentdir[offset-count-1] = '\0';		//parent path
							//printf("\n%s\n%d\n%d",currentdir,count,offset);
							if(strcmp(dirname,"../\0")!=0)
								strcat(currentdir,dirname+2);

						}
						else
							strcat(currentdir,dirname+2);
					}
					else if(dirname[0]=='/')
					{
						strcpy(currentdir,dirname);
					}
					else
					{
						if(strcmp(currentdir,"/\0") != 0)
							strcat(currentdir,"/\0");
						strcat(currentdir,dirname);
					}
				}
				else 	//
				{
					memcpy(currentdir,"/\0",sizeof("/\0"));
				}
				offset = 0;
				count = 0;
				//printf("curretdirrrrrrrrrrrrrrrrrrrrrrrrrr:         %s\n",currentdir);
				while(currentdir[offset] != '\0')
				{
					memset(tempdir,'\0',80);	
					if(currentdir[offset]=='.')
					{
						
						//printf("curretdirrrrrrrrrrrrrrrrrrrrrrrrrr:         %s\n",currentdir);
						//printf("ttttttttttttttttttttttttttttttttttttttttt :%s\n",tempdir);
						if(currentdir[offset+1]=='.')
						{
							memcpy(tempdir,currentdir,offset-1);
							offset++;
							count2=0;
							offset2=0;
							//printf("tempppppppppppppppppppppppdir1111111 %s\n",tempdir); 
							while(tempdir[offset2]!='\0')
							{
								if(tempdir[offset2]=='/')
									count2=0;
								else 
									count2++;
								offset2++;
							}
							//printf("tempppppppppppppppppppppppdir2222222 :%s\n",tempdir); 
							tempdir[offset2-count2-1]='\0';
							strcat(tempdir,"/\0");	
						}
						else	
							memcpy(tempdir,currentdir,offset);
						if(strlen(tempdir)==0)
							strcpy(tempdir,"/\0");
						strcat(tempdir,currentdir+offset+2);
						strcpy(currentdir,tempdir);
						offset = -1;	//set offset = 0 ( next will do offset++,so offset = -1)
					}
					offset++;
				}
				while(currentdir[offset]!='\0')
				{
					offset++;
				}
				if(currentdir[offset-1]=='/')
					currentdir[offset-1]='\0';
				if(strlen(currentdir)==0)
					strcpy(currentdir,"/\0");
				offset=0;
				fcbtouser(cfcb,openfilelist,currentdir);
				//printf("cfcb name %s\n",cfcb->filename);
				//printf("cfcb len %ld\n",cfcb->length);
				//printf("cfcb first:%d\n",cfcb->first);
			}
			else 
				printf("\"%s\":Not a directory\n",dirname);
			
		}
		else 
		{
			printf("%s:no such file or dircetory5",dirname);
		}
	}
}

void my_mkdir(char *dirname)
{
	int offset = 0;
	int count = 0;
	int i,j;
	unsigned long totallen=BLOCKSIZE;		//current file length
	unsigned long curlen=2*sizeof(fcb);
	int error = 0;
	char subdir[12];
	unsigned short fb;		//file start block
	unsigned short pfb;		//parrent start block
	fcb *f;
	fcb *pf;	//parent fcb;
	fcb *root;	//new directory root
	fb = openfilelist[0].first;	//first block
	pfb = fb;	//parent first block
	//printf("mkdir fb: %d\n",fb);
	pf = (fcb*)mydisk->data[fb];  	
	while(*(dirname+offset) != '\0')
	{
		if(*(dirname+offset) == '/') //get the directory name
		{
			memcpy(subdir,dirname+offset-count,count);
			subdir[count] = '\0';
			count = 0;
			if(strlen(subdir)==0)	//root directory
			{
				fb = ((fcb*)mydisk->data[mydisk->boot.root])->first;	//root directory
				memcpy(subdir,".\0",sizeof(".\0")); 
				pfb = fb;	
			}
			//printf("%s\n",subdir);
			f = (fcb*)mydisk->data[fb];
			//printf("first:%d\n",f->first);
			if(f->attribute==0)	//directory
			{
				if(f->free==1)	//fcb is not free
				{
					if(f->length < BLOCKSIZE)	//search current block
					{
						for(i=0; i< (int)(f->length/sizeof(fcb));i++)
						{
							if(strcmp(subdir,(f+i)->filename)==0)	//found the file or dirctory
							{	
								pf = f+i;	//parent fcb
								fb = (f+i)->first;
								pfb = fb;
								error = 1;
								break;
							}
							error = 0;
						}
					}
					else	//serach all blocks
					{
						for(i=0;i<(int)(f->length/BLOCKSIZE);i++)
						{
							for(j=0; j < (BLOCKSIZE/sizeof(fcb));j++) //search  blocks
							{
								if(strcmp(subdir,(f+j)->filename)==0)
								{
									pf = f+j;
									fb = (f+j)->first;
									pfb = fb;
									error = 1;
									break;
								}
								error = 0;
							}
							if(error)
								break;
							else
							{
								fb = mydisk->fat1[fb].id;
								f = (fcb*)mydisk->data[fb];
							}
						}
						if(!error)
						{
							for(i=0; i< (int)((f->length%BLOCKSIZE)/sizeof(fcb));i++)
							{
								if(strcmp(subdir,(f+i)->filename)==0)
								{
									pf = f+i;
									fb = (f+i)->first;
									pfb = fb;
									error = 1;
									break;
								}
								error = 0;
							}
						}
					}
					/*for(i=0;i<(int)(((fcb*)mydisk->data[fb])->length/sizeof(fcb));i++)
					{
						if(strcmp(subdir,((fcb*)(mydisk->data[fb]+i))->filename)==0)
						{
							pfb = fb;
							pf = ((fcb*)(mydisk->data[fb]+i));
							fb = pf->first;
							error = 1;		//find parent directory
							break;
						}
						error = 0;
					}*/
				}
				else
				{
					error = 0;	//directory is free
					printf("%s,no such file or dirctory\n",dirname);
				}
			}
			else
			{
				error = 0; //is not directory
				printf("current file is not directory,please close file\n");
			}
		}
		else
		{
			error = 1;
			count++;
		}
		if(!error)
				break;		//can't find parent direcoty
		offset++;
	}
	strcpy(subdir,dirname+offset-count);
	//printf("dirname1:%s\n",subdir);
	//printf("mkdir error %d\n",error);
	//printf("offset:%d\t count:%d\n",offset, count);
	if(error)	//have is file path 
	{
		error = 0;
		f = (fcb*)mydisk->data[fb];
		if(f->attribute==0)
		{
			if(f->length < BLOCKSIZE)		//search dircetory
			{
				for(i=0; i< (int)(f->length/sizeof(fcb));i++)
				{
					if(strcmp(subdir,(f+i)->filename)==0)
					{
						//pfb = fb;
						//fb = (f+i)->first;
						error = 1;
						break;
					}
					error = 0;
				}
			}
			else
			{
				for(i=0;i<(int)(f->length/BLOCKSIZE);i++)
				{
					for(j=0; j < (BLOCKSIZE/sizeof(fcb));j++)
					{
						if(strcmp(subdir,(f+j)->filename)==0)
						{
							//pfb = fb;
							//fb = (f+j)->first;
							error = 1;
							break;
						}
						error = 0;
					}
					if(error)
						break;
					else
					{
						fb = mydisk->fat1[fb].id;
						f = (fcb*)mydisk->data[fb];
					}
				}
				if(error)	//found dirctory
				{
					printf("%s:file is exists1\n",dirname);
					return;
				}
				else
				{
					for(j=0; j < (int)((f->length%BLOCKSIZE)/sizeof(fcb));j++)
					{
						if(strcmp(subdir,(f+j)->filename)==0)
						{
							//pfb = fb;
							//fb = (f+j)->first;
							error = 1;
							break;
						}
						error = 0;
					}
				}
					
				
			}
			if(error)	//file exists
			{
				printf("%s:file is exists2\n",dirname);
				return;
			}
			else	//create new direcoty
			{
				
				
				for(i=fb;mydisk->fat1[i].id != END;i=mydisk->fat1[i].id) //calculating the total area
				{
					//printf("%ld\n",totallen);
					//printf("%d",i);
					totallen += BLOCKSIZE;
				}
				f = (fcb*)mydisk->data[fb];
				//printf("fb:%d\n",fb);
				curlen = f->length;
				//printf("%s\n",f->filename);
				//printf("curlen1 %ld\n",f->length);
				if( (curlen + sizeof(fcb)) > totallen)		
				{
					i = my_malloc();   //malloc memroy
					if(i != -1)
					{
						f->length += BLOCKSIZE;
						(mydisk->fat1+fb)->id = i;
						fb = i;
					}
					else	//malloc error
					{
						printf("virtual memmory if full\n");
						return;
					}
				}
				for(i = curlen / BLOCKSIZE; i>0; i--)	//locate free block;
					fb = mydisk->fat1[fb].id;		
				
				i = my_malloc(); //for new directory malloc area
				if(i != -1)	//malloc success
				{
					f = (fcb*)mydisk->data[fb]; //located free block head
					f = f+((curlen%BLOCKSIZE)/sizeof(fcb));	//located write point
					//printf("curlen:%ld,size of fcb:%ld,%ld,%ld,%ld\n",curlen,sizeof(fcb),sizeof(char),sizeof(unsigned short),sizeof(unsigned long));
					strcpy(f->filename,subdir);
					//printf("filename: %s",f->filename);
					f->attribute = 0;
					f->first = i;
					f->free = 1;
					f->length = 2*sizeof(fcb);
					setdate(f);
					pf->length += sizeof(fcb);	//modify parent fcb
					((fcb*)mydisk->data[pfb])->length = pf->length;	//modify "./" fcb
					if(pfb==0)
						((fcb*)mydisk->data[pfb]+1)->length = pf->length;
					//printf("pf name%s\n",pf->filename);
					//printf("pfb name %s\n",((fcb*)mydisk->data[pfb])->filename);
					//printf("pfb data %d\n",pfb);
					
					root = (fcb*)mydisk->data[i];	//create "./" and "../" directory

					//printf("i data%d\n",i);
					memcpy(root,f,sizeof(fcb));
					//printf("root length %ld\n",root->length);
					memcpy(root->filename,".\0",sizeof(".\0"));
					memcpy(root+1,pf,sizeof(fcb));
					memcpy((root+1)->filename,"..\0",sizeof("..\0"));
					//printf("directory success\n");
					f = (fcb*)mydisk->data[i];
					//printf("f info %s, len %ld\n",f->filename,f->length);
					//printf("root info name%s, len %ld\n",root->filename,root->length);
				}
				else
				{
					printf("virtual memmory if full\n");
					return;
				}	
			}
			
		}
		else 	
		{
			printf("current file is not directory,please close file\n");
		}
	}
}

void my_rmdir(char *dirname)
{
	int count = 0;
	int offset = 0;
	char tempdir[80]; //copy the currentdir
	fcb *cfcb;
	fcb *pfcb;
	if(strlen(dirname)==0)
	{
		printf("my_rmdir:missing file path\n");
		return;
	}
	else if(strcmp(dirname,"./\0")==0)
	{
		printf("fail to remove \"./\": Invalid arguemnt\n");
	}
	else 
	{
		if(strcmp(dirname,"/\0") != 0 || (strcmp(currentdir,"/\0")==0 && (strcmp(dirname,"./\0")==0 || strcmp(dirname,"../\0")==0 ))!=0 )
		{
			cfcb = search(dirname);
			//printf("namename:%s\n",cfcb->filename);
			if(cfcb != NULL)
			{
				if(cfcb->attribute == 0)	//directory
				{
					if(cfcb->length == 2*sizeof(fcb))	//delete fcb; modify the parent info;free area
					{
						strcpy(tempdir,currentdir);
						if(dirname[0]=='/')	//absolute file path
						{
							while(*(dirname+offset) != '\0') //calculate the offset and parent file path
							{
								if(*(dirname+offset)=='/')
									count = 0;
								else
									count++;
								offset++;
							}
							*(dirname+offset-count) = '\0'; // parent file path
							pfcb = search(dirname);	//parent fcb	
							//printf("//////name:%s\n",pfcb->filename);
						}
						else	//relative file path
						{
							
							count = 0;
							offset = 0;
							if(dirname[0]=='.')  // file path "./..." or "../" or "../..."
							{
								//printf("type of ./.../\n");
								if(dirname[1] == '.')
								{
									while(*(tempdir+offset) != '\0')
									{
										if(*(tempdir+offset) == '/')
											count =0;
										else 
											count++;
										offset++;
									}
									tempdir[offset-count-1] = '\0';	//currentdir parent path
									if(strcmp(dirname,"../\0")!=0)
										strcat(tempdir,dirname+2);
									else 
										strcat(tempdir,dirname+3);	
								}
								else //file path "./...."
								{
									strcat(tempdir,dirname+1);
								}
							}
							else	//path type "...../"
							{
								strcat(tempdir,"/\0");
								strcat(tempdir,dirname);	//absolute path
								
							}
							strcpy(dirname,tempdir);	//operation file path
							offset = 0;
							count = 0;
							while(*(tempdir+offset) != '\0') //calculate the offset and parent file path
							{
								if(*(tempdir+offset)=='/')
									count = 0;
								else
									count++;
								offset++;
							}
							//printf("\n\n%s\n\n",tempdir);
							*(tempdir+offset-count) = '\0'; // parent file path
							pfcb = search(tempdir);
							//printf("...////%s\n",pfcb->filename);
						}
						offset = 0;
						while(*(dirname+offset)!='\0')
						{
							offset++;
						}
						//printf("strp 1111\n");
						my_free(cfcb,pfcb);
						//printf("strp 2222\n");
						if(dirname[offset-1]=='/')
							dirname[offset-1]=='\0';	// delete the last '/'
						if(strcmp(dirname,currentdir)==0)	//rmdir the current dir
						{
							strcpy(currentdir,tempdir);	//current path = parent path
							fcbtouser(pfcb,openfilelist,currentdir);	//modify current directpry
						}
						
					}
					else
					{
						printf("%s:dirctory is not empty\n",dirname);
					}
				}
				else
				{
					printf("%s:file is not directory\n",dirname);
				}
			}
			else
			{
				printf("%s:no such dirctory\n",dirname);
			}
		}
		else
		{
			printf("cant modify root dirctory\n");
		}
	}
}

int my_create(char *dirname)
{
	int offset = 0;
	int count = 0;
	int i,j;
	unsigned long totallen=BLOCKSIZE;		//current file length
	unsigned long curlen=2*sizeof(fcb);
	int error = 0;
	char subdir[12];
	unsigned short fb;		//file start block
	unsigned short pfb;		//parrent start block
	fcb *f;
	fcb *pf;	//parent fcb;
	fcb *root;	//new directory root
	fb = openfilelist[0].first;	//first block
	pfb = fb;	//parent first block
	//printf("mkdir fb: %d\n",fb);
	pf = (fcb*)mydisk->data[fb];  	
	while(*(dirname+offset) != '\0')
	{
		if(*(dirname+offset) == '/') //get the directory name
		{
			memcpy(subdir,dirname+offset-count,count);
			subdir[count] = '\0';
			count = 0;
			if(strlen(subdir)==0)	//root directory
			{
				fb = ((fcb*)mydisk->data[mydisk->boot.root])->first;	//root directory
				memcpy(subdir,".\0",sizeof(".\0")); 
				pfb = fb;	
			}
			//printf("%s\n",subdir);
			f = (fcb*)mydisk->data[fb];
			//printf("first:%d\n",f->first);
			if(f->attribute==0)	//directory
			{
				if(f->free==1)	//fcb is not free
				{
					if(f->length < BLOCKSIZE)	//search current block
					{
						for(i=0; i< (int)(f->length/sizeof(fcb));i++)
						{
							if(strcmp(subdir,(f+i)->filename)==0)	//found the file or dirctory
							{	
								pf = f+i;	//parent fcb
								fb = (f+i)->first;
								pfb = fb;
								error = 1;
								break;
							}
							error = 0;
						}
					}
					else	//serach all blocks
					{
						for(i=0;i<(int)(f->length/BLOCKSIZE);i++)
						{
							for(j=0; j < (BLOCKSIZE/sizeof(fcb));j++) //search  blocks
							{
								if(strcmp(subdir,(f+j)->filename)==0)
								{
									pf = f+j;
									fb = (f+j)->first;
									pfb = fb;
									error = 1;
									break;
								}
								error = 0;
							}
							if(error)
								break;
							else
							{
								fb = mydisk->fat1[fb].id;
								f = (fcb*)mydisk->data[fb];
							}
						}
						if(!error)
						{
							for(i=0; i< (int)((f->length%BLOCKSIZE)/sizeof(fcb));i++)
							{
								if(strcmp(subdir,(f+i)->filename)==0)
								{
									pf = f+i;
									fb = (f+i)->first;
									pfb = fb;
									error = 1;
									break;
								}
								error = 0;
							}
						}
					}
					/*for(i=0;i<(int)(((fcb*)mydisk->data[fb])->length/sizeof(fcb));i++)
					{
						if(strcmp(subdir,((fcb*)(mydisk->data[fb]+i))->filename)==0)
						{
							pfb = fb;
							pf = ((fcb*)(mydisk->data[fb]+i));
							fb = pf->first;
							error = 1;		//find parent directory
							break;
						}
						error = 0;
					}*/
				}
				else
				{
					error = 0;	//directory is free
					printf("%s,no such file or dirctory\n",dirname);
				}
			}
			else
			{
				error = 0; //is not directory
				printf("current file is not directory,please close file\n");
			}
		}
		else
		{
			error = 1;
			count++;
		}
		if(!error)
				break;		//can't find parent direcoty
		offset++;
	}
	strcpy(subdir,dirname+offset-count);
	//printf("dirname1:%s\n",subdir);
	//printf("mkdir error %d\n",error);
	//printf("offset:%d\t count:%d\n",offset, count);
	if(error)	//have is file path 
	{
		error = 0;
		f = (fcb*)mydisk->data[fb];
		if(f->attribute==0)
		{
			if(f->length < BLOCKSIZE)		//search dircetory
			{
				for(i=0; i< (int)(f->length/sizeof(fcb));i++)
				{
					if(strcmp(subdir,(f+i)->filename)==0)
					{
						//pfb = fb;
						//fb = (f+i)->first;
						error = 1;
						break;
					}
					error = 0;
				}
			}
			else
			{
				for(i=0;i<(int)(f->length/BLOCKSIZE);i++)
				{
					for(j=0; j < (BLOCKSIZE/sizeof(fcb));j++)
					{
						if(strcmp(subdir,(f+j)->filename)==0)
						{
							//pfb = fb;
							//fb = (f+j)->first;
							error = 1;
							break;
						}
						error = 0;
					}
					if(error)
						break;
					else
					{
						fb = mydisk->fat1[fb].id;
						f = (fcb*)mydisk->data[fb];
					}
				}
				if(error)	//found dirctory
				{
					printf("%s:file is exists1\n",dirname);
					return 0;
				}
				else
				{
					for(j=0; j < (int)((f->length%BLOCKSIZE)/sizeof(fcb));j++)
					{
						if(strcmp(subdir,(f+j)->filename)==0)
						{
							//pfb = fb;
							//fb = (f+j)->first;
							error = 1;
							break;
						}
						error = 0;
					}
				}
					
				
			}
			if(error)	//file exists
			{
				printf("%s:file is exists2\n",dirname);
				return 0;
			}
			else	//create new direcoty
			{
				
				
				for(i=fb;mydisk->fat1[i].id != END;i=mydisk->fat1[i].id) //calculating the total area
				{
					//printf("%ld\n",totallen);
					//printf("%d",i);
					totallen += BLOCKSIZE;
				}
				f = (fcb*)mydisk->data[fb];
			//	printf("fb:%d\n",fb);
				curlen = f->length;
				//printf("%s\n",f->filename);
				//printf("curlen1 %ld\n",f->length);
				if( (curlen + sizeof(fcb)) > totallen)		
				{
					i = my_malloc();   //malloc memroy
					if(i != -1)
					{
						f->length += BLOCKSIZE;
						(mydisk->fat1+fb)->id = i;
						fb = i;
					}
					else	//malloc error
					{
						printf("virtual memmory if full\n");
						return 0;
					}
				}
				for(i = curlen / BLOCKSIZE; i>0; i--)	//locate free block;
					fb = mydisk->fat1[fb].id;		
				
				i = my_malloc(); //for new directory malloc area
				if(i != -1)	//malloc success
				{
					f = (fcb*)mydisk->data[fb]; //located free block head
					f = f+((curlen%BLOCKSIZE)/sizeof(fcb));	//located write point
					//printf("curlen:%ld,size of fcb:%ld,%ld,%ld,%ld\n",curlen,sizeof(fcb),sizeof(char),sizeof(unsigned short),sizeof(unsigned long));
					strcpy(f->filename,subdir);
					//printf("filename: %s",f->filename);
					setdate(f);
					f->attribute = 1;
					f->first = i;
					f->free = 1;
					f->length = 0;
					f->right = 1;
					pf->length += sizeof(fcb);	//modify parent fcb
					((fcb*)mydisk->data[pfb])->length = pf->length;	//modify "./" fcb
					if(pfb==0)
						((fcb*)mydisk->data[pfb]+1)->length = pf->length;	//modify "../" fcb
					fcbtouser(pf,openfilelist,currentdir);
					//printf("pf name%s\n",pf->filename);
					//printf("pfb name %s\n",((fcb*)mydisk->data[pfb])->filename);
					//printf("pfb data %d\n",pfb);
					printf("create file success\n");
				}
				else
				{
					printf("virtual memmory if full\n");
					return 0;
				}	
			}
			
		}
		else 	
		{
			printf("current file is not directory,please close file\n");
		}
	}
	return 0;	
}

void my_rm(char *dirname)
{
	int count = 0;
	int offset = 0;
	char tempdir[80]; //copy the currentdir
	fcb *cfcb;
	fcb *pfcb;
	if(strlen(dirname)==0)		//no argument
	{
		printf("my_rm:missing file path\n");
		return;
	}
	else if(strcmp(dirname,"./\0")==0)
	{
		printf("fail to remove \"./\": IS a directory\n");
		return;
	}
	else 
	{
		if(strcmp(dirname,"/\0") != 0 || (strcmp(currentdir,"/\0")==0 && (strcmp(dirname,"./\0")==0 || strcmp(dirname,"../\0")==0 ))!=0 )
		{
			cfcb = search(dirname);
			//printf("namename:%s\n",cfcb->filename);
			if(cfcb != NULL)
			{
				if(cfcb->attribute != 0)	//is file
				{
					strcpy(tempdir,currentdir);
					if(dirname[0]=='/')	//absolute file path
					{
						while(*(dirname+offset) != '\0') //calculate the offset and parent file path
						{
							if(*(dirname+offset)=='/')
								count = 0;
							else
								count++;
							offset++;
						}
						*(dirname+offset-count) = '\0'; // parent file path
						pfcb = search(dirname);	//parent fcb	
						//printf("//////name:%s\n",pfcb->filename);
					}
					else	//relative file path
					{
						
						count = 0;
						offset = 0;
						if(dirname[0]=='.')  // file path "./..." or "../" or "../..."
						{
							//printf("type of ./.../\n");
							if(dirname[1] == '.')
							{
								while(*(tempdir+offset) != '\0')
								{
									if(*(tempdir+offset) == '/')
										count =0;
									else 
										count++;
									offset++;
								}
								tempdir[offset-count-1] = '\0';	//currentdir parent path
								if(strcmp(dirname,"../\0")!=0)
									strcat(tempdir,dirname+2);
								else 
									strcat(tempdir,dirname+3);	
							}
							else //file path "./...."
							{
								strcat(tempdir,dirname+1);
							}
						}
						else	//path type "...../"
						{
							if(strcmp(currentdir,"/\0")!=0)
								strcat(tempdir,"/\0");
							strcat(tempdir,dirname);	//absolute path
							
						}
						strcpy(dirname,tempdir);	//operation file path
						offset = 0;
						count = 0;
						while(*(tempdir+offset) != '\0') //calculate the offset and parent file path
						{
							if(*(tempdir+offset)=='/')
								count = 0;
							else
								count++;
							offset++;
						}
						//printf("\n\n%s\n\n",tempdir);
						*(tempdir+offset-count) = '\0'; // parent file path
						pfcb = search(tempdir);
						//printf("...////%s\n",pfcb->filename);
					}
					offset = 0;
					while(*(dirname+offset)!='\0')
					{
						offset++;
					}
					if(dirname[offset-1]=='/')
						dirname[offset-1]=='\0';	// delete the last '/'
					for(offset=1;offset<MAXOPENFILE;offset++)	//checking the openfilelist
					{
						if(strcmp(openfilelist[offset].dir,dirname)==0)
						{
							printf("\"%s\":file was opened,can not remove\n",dirname);
							return;
						}
					} 
					my_free(cfcb,pfcb);	//delete file,modify the parent fcb
				}
				else
				{
					printf("can not remove \"%s\": is a directory\n",dirname);
				}
			}
			else
			{
				printf("%s:no such dirctory\n",dirname);
			}
		}
		else
		{
			printf("cant not rm \"%s\": Is a dirctory\n",dirname);
		}
	}
}


int my_malloc()
{
	unsigned short i;
	for(i=0;i<SIZE/BLOCKSIZE-5;i++)
	{
		if((mydisk->fat1+i)->id == FREE)
		{
			(mydisk->fat1+i)->id = END;
			return i;
		}
	}
	return -1;
}

void my_free(fcb *f,fcb *pf)	//free fcb and parent fcb
{
	//printf("my_free:fb,%d, pfb:%d\n",f->first,pf->first);
	unsigned short fb; //file block index
	unsigned short ffb; //free block index
	char *head;
	fcb *temp;
	fb = f->first;
	memcpy(f->filename,"\0",sizeof(fcb));
	f->free = 0;	
	f->length = 0;
	f->first = FREE;	
	pf->length -= sizeof(fcb);	//parent length
	((fcb*)mydisk->data[pf->first])->length = pf->length;	// "./" length
	//printf("id;%d\n",fb);
	//printf("format 0\n");
	
	while(mydisk->fat1[fb].id != END)	//format all file blocks and free	
	{
		//printf("id;%d\n",fb);
		head = mydisk->data[fb];
		memset(head,0,BLOCKSIZE);	//free BLOCK
		ffb = fb;
		fb = mydisk->fat1[fb].id;	//next block
		mydisk->fat1[ffb].id = FREE;	//free area
	}
	head = mydisk->data[fb];
	memset(head,0,BLOCKSIZE);	//free BLOCK
	mydisk->fat1[fb].id = FREE;	//free area
	
	//printf("format ok\n");
	
	fb = pf->first;
	while(mydisk->fat1[fb].id != END)
	{
		temp = (fcb*)mydisk->data[fb] + BLOCKSIZE/sizeof(fcb);	//end of block
		//printf("fcb number:%ld\n",temp-f-1);
		memcpy(f,f+1,sizeof(fcb)*(temp-f-1));
		fb = mydisk->fat1[fb].id;	//next block index
		f = (fcb*)mydisk->data[fb];	//next block
		memcpy(temp-1,f,sizeof(fcb));	//modify the previous tabel last directory = next tabel first directory entry
	} 
	temp = (fcb*)mydisk->data[fb] + BLOCKSIZE/sizeof(fcb);	//end of block
	//printf("fcb number:%ld\n",temp-f-1);
	memcpy(f,f+1,sizeof(fcb)*(temp-f-1));
	temp = temp - 1;
	strcpy(temp->filename,"\0");
	temp->first = FREE;
	temp->free = 0;
	temp->length = 0;	 
}


int my_open(char *dirname)	//sucess return the fd
{
	int count = 0;
	int offset = 0;
	char tempdir[80]; //copy the currentdir
	char absdir[80]; 	//absoluted file path
	fcb *cfcb;
	fcb *pfcb;
	strcpy(absdir,currentdir);
	if(strlen(dirname)==0)		//no argument
	{
		printf("my_open:missing file path\n");
		return 0;
	}
	else if(strcmp(dirname,"./\0")==0)
	{
		printf("fail to open \"./\": IS a directory\n");
		return 0;
	}
	else 
	{
		if(strcmp(dirname,"/\0") != 0 || (strcmp(absdir,"/\0")==0 && (strcmp(dirname,"./\0")==0 || strcmp(dirname,"../\0")==0 ))!=0 )
		{
			cfcb = search(dirname);
			//printf("namename:%s\n",cfcb->filename);
			if(cfcb != NULL)
			{
				if(cfcb->attribute != 0)	//is file
				{
					strcpy(tempdir,absdir);
					if(dirname[0]=='/')	//absolute file path
					{
						while(*(dirname+offset) != '\0') //calculate the offset and parent file path
						{
							if(*(dirname+offset)=='/')
								count = 0;
							else
								count++;
							offset++;
						}
						*(dirname+offset-count) = '\0'; // parent file path
						pfcb = search(dirname);	//parent fcb	
						//printf("//////name:%s\n",pfcb->filename);
					}
					else	//relative file path
					{
						
						count = 0;
						offset = 0;
						if(dirname[0]=='.')  // file path "./..." or "../" or "../..."
						{
							//printf("type of ./.../\n");
							if(dirname[1] == '.')
							{
								while(*(tempdir+offset) != '\0')
								{
									if(*(tempdir+offset) == '/')
										count =0;
									else 
										count++;
									offset++;
								}
								tempdir[offset-count-1] = '\0';	//currentdir parent path
								if(strcmp(dirname,"../\0")!=0)
									strcat(tempdir,dirname+2);
								else 
									strcat(tempdir,dirname+3);	
							}
							else //file path "./...."
							{
								strcat(tempdir,dirname+1);
							}
						}
						else	//path type "...../"
						{
							if(strcmp(absdir,"/\0")!=0)
								strcat(tempdir,"/\0");
							strcat(tempdir,dirname);	//absolute path
							
						}
						strcpy(dirname,tempdir);	//operation file path
						//printf("\n\n%s\n\n",tempdir);
					}
					offset = 0;
					while(*(dirname+offset)!='\0')
					{
						offset++;
					}
					if(dirname[offset-1]=='/')
						dirname[offset-1]=='\0';	// delete the last '/'
					//printf("userfile>>%s\n",(openfilelist+curdir)->filename);
					for(offset=1;offset<MAXOPENFILE;offset++)	//checking the openfilelist
					{
						if(strcmp(openfilelist[offset].dir,dirname)==0)
						{
							printf("\"%s\":file already opened\n",dirname);
							return offset;
						}
					}
					for(offset=1;offset<MAXOPENFILE;offset++)	//allocate open  file list
					{
						if(openfilelist[offset].topenfile==0)
						{
							openfilelist[offset].topenfile==1;	
							fcbtouser(cfcb,openfilelist+offset,tempdir);
							printf("\"%s\":open success\n",tempdir);
							return offset;
						}
					}
					printf("open file error: user open file list is full\n"); 
				}
				else
				{
					printf("can not open \"%s\": is a directory\n",dirname);
				}
			}
			else
			{
				printf("open error \"%s\":no such dirctory\n",dirname);
			}
		}
		else
		{
			printf("cant not open \"%s\": Is a dirctory\n",dirname);
		}
	}
	return 0;
}

void my_close(char fd)
{
	fcb *f;
	int i;
	i = fd-'0';
	if(i>0 && i < MAXOPENFILE)
	{
		if(openfilelist[i].topenfile!=0)
		{
			if(openfilelist[i].fcbstate==1)	//write back to 
			{
				f = search(openfilelist[i].dir);
				f->length = openfilelist[i].length;
				f->right = openfilelist[i].right;
			}
			openfilelist[i].topenfile=0;
			openfilelist[i].length = 0;
			strcpy(openfilelist[i].filename,"\0");
			strcpy(openfilelist[i].dir,"\0");
		}
		else
		{
			printf("error,don't have this file\n");
		}
	}
	else
	{
		printf("Invalid argument\n");
	}
}

int my_write(char fd)
{
	int i,len,index;
	char wstyle;
	char text[BLOCKSIZE*10];
	i = fd-'0';
	if(i>0 && i<MAXOPENFILE)
	{
		if(openfilelist[i].topenfile!=0)
		{
			if(openfilelist[i].right==1)
			{
				printf("please choice write style:add:\"a\",cover:\"c\",truncation:\"t\": \n");
				scanf("%s",&wstyle);
				while(( getchar())!='\n');	//clear buffer
				if(wstyle != 'a' && wstyle != 'c' && wstyle != 't')
				{
					printf("Invalid argument\n");
					return -1;			
				}
				index=0;
				while( (text[index]=getchar()) != EOF)
				{
					printf("%c\n",text[index]);
					index++;
				}
				//printf("over1\n");
				rewind(stdin);
				//while(( getchar())==EOF)	//clear buffer
					//printf("over2\n");
				//printf("over3\n");
				//printf("over\n");
				//sleep(5);
				len = do_write(i,text,index,wstyle);
				return len;
			}
			else
			{
				printf("do not have authority\n");
				return -1;
			}
		}
		else 
		{
			printf("don't have this file\n");		
		}
	}
	else
	{
		printf("Invalid argument\n");
		return -1;
	}
}

int do_write(int fd,char *text, int len, char wstyle)
{
	int offset;
	int count;
	int page;
	int freepage;
	int i=0;
	int templen = 0;
	int block;
	char *head;
	char buff[BLOCKSIZE];
	page=openfilelist[fd].first;
	offset = openfilelist[fd].length % BLOCKSIZE;
	count = openfilelist[fd].length / BLOCKSIZE;
	//printf("\nadd\n");
	if(wstyle=='a')	//add 
	{
		//printf("\nadd2\n");
		for(i=0;i<count;i++)	//found the write page
		{
			if(mydisk->fat1[page].id==END)	//
			{
				block = my_malloc(); //malloc area
				if(block != -1)
				{
					mydisk->fat1[page].id=block;
					page = block;
				}
				else 
					printf("virtual memory is full\n");
					//printf("malloc block:%d\n",block);
			}
			else
				page = mydisk->fat1[page].id;
		}
		//printf("\nadd3\n");
		head = mydisk->data[page];
		memcpy(buff,head,BLOCKSIZE);	//read last block to buffer
		//printf("\nadd4\n");
		if(len>BLOCKSIZE || (len<BLOCKSIZE && len>(BLOCKSIZE-offset) )) // write many blocks
		{
			
			//printf("\nadd5\n");
			printf("%d\n",page);
			memcpy(buff+offset,text,BLOCKSIZE-offset);
			memcpy(head,buff,BLOCKSIZE);  
			//printf("\nadd6\n");
			if(mydisk->fat1[page].id==END)	//if the end block
			{
				block = my_malloc(); //malloc area
				if(block != -1)
				{
					mydisk->fat1[page].id=block;
					page = block;
				}
				else 
					printf("virtual memory is full\n");
					//printf("malloc block:%d\n",block);
			}
			else
			{
				page = mydisk->fat1[page].id; //next block
			}
			
			templen = BLOCKSIZE-offset; //record have written length  
			len = len - BLOCKSIZE + offset;
			offset = len % BLOCKSIZE; 	//new offset and count
			count = len / BLOCKSIZE;
			//printf("321\n");
			i=0;	//index = 0
			if(len>BLOCKSIZE)
			{
				for(i=0;i<count;i++)
				{
					head = mydisk->data[page];
					memcpy(buff,text+i*BLOCKSIZE+templen,BLOCKSIZE);	//write into buffer
					memcpy(head,buff,BLOCKSIZE);	//write into virtual memory
					if(mydisk->fat1[page].id==END)	//
					{
						block = my_malloc(); //malloc area
						if(block != -1)
						{
							mydisk->fat1[page].id=block;
							page = block;
						}
						else 
							printf("virtual memory is full\n");
						//printf("malloc block:%d\n",block);
					}
					else
					{
						page = mydisk->fat1[page].id;
					}
					head = mydisk->data[page];
				}
			}
			//printf("%d",page);
			memset(buff,'\0',BLOCKSIZE); //buffer
			memcpy(buff,text+i*BLOCKSIZE+templen,len-i*BLOCKSIZE);	//copy to buffer 
			memcpy(head,buff,BLOCKSIZE);	//write to virtual memory
			openfilelist[fd].length += len+templen;	//update length
			openfilelist[fd].fcbstate = 1;	//file fcb was modified
		}
		else	//write less one block
		{
			//printf("\nadd5\n");
			memcpy(buff+offset,text,len);
			memcpy(head,buff,BLOCKSIZE);
			openfilelist[fd].length += len;
			openfilelist[fd].fcbstate = 1;
		} 		
	}
	else if(wstyle=='c')	//cover
	{
		//printf("\ncover\n");
		offset = openfilelist[fd].rw % BLOCKSIZE;
		count = openfilelist[fd].rw / BLOCKSIZE;
		for(i=0;i<count;i++)	//found the write page
		{
			page = mydisk->fat1[page].id;
		}
		head = mydisk->data[page];
		memcpy(buff,head,BLOCKSIZE);	//read last block to buffer
		if(len>BLOCKSIZE || (len<BLOCKSIZE && len>(BLOCKSIZE-offset) )) // write many blocks
		{
			memcpy(buff+offset,text,BLOCKSIZE-offset);
			memcpy(head,buff,BLOCKSIZE);  
			if(mydisk->fat1[page].id==END)	//if the end block
			{
				block = my_malloc(); //malloc area
				if(block != -1)
				{
					mydisk->fat1[page].id=block;
					page = block;
				}
				else 
					printf("virtual memory is full\n");
					//printf("malloc block num:%d\n",block);
			}
			else
			{
				page = mydisk->fat1[page].id; //next block
			}
			
			len = len - BLOCKSIZE + offset;
			offset = len % BLOCKSIZE; 	//new offset and count
			count = len / BLOCKSIZE;
			i=0;	//index = 0
			if(len>BLOCKSIZE)
			{
				count = len/BLOCKSIZE;
				for(i=0;i<count;i++)
				{
					head = mydisk->data[page];
					memcpy(buff,text+i*BLOCKSIZE,BLOCKSIZE);	//write into buffer
					memcpy(head,buff,BLOCKSIZE);	//write into virtual memory
					if(mydisk->fat1[page].id==END)	//
					{
						block = my_malloc(); //malloc area
						if(block != -1)
						{
							mydisk->fat1[page].id=block;
							page = block;
						}
						else 
							printf("virtual memory is full\n");
							//printf("malloc block num:%d\n",block);
					}
					else
					{
						page = mydisk->fat1[page].id;
					}
					head = mydisk->data[page];
				}
			}
			memset(buff,'\0',BLOCKSIZE); //flush buffer
			memcpy(buff,text+i*BLOCKSIZE,len-i*BLOCKSIZE);	//copy to buffer 
			memcpy(head,buff,BLOCKSIZE);	//write to virtual memory
			openfilelist[fd].fcbstate = 1;	//file fcb was modified
			len = len+BLOCKSIZE-offset;	//write length
			if(len>(openfilelist[fd].length-openfilelist[fd].rw))
			{
				openfilelist[fd].length = len + openfilelist[fd].rw;
			}
			
		}
		else	//write less one block
		{
			memcpy(buff+offset,text,len);
			memcpy(head,buff,BLOCKSIZE);
			if(len>(openfilelist[fd].length-openfilelist[fd].rw))
			{
				openfilelist[fd].length = len + openfilelist[fd].rw;
			}
			openfilelist[fd].fcbstate = 1;
		} 	
	}
	else if(wstyle=='t')	//truncation
	{
		offset = openfilelist[fd].length % BLOCKSIZE;
		count = openfilelist[fd].length / BLOCKSIZE;
		//for(i=0;i<=count;i++)  //clear content
		//{
		head = mydisk->data[page];
		//printf("page number :%d\n",page);
		//	page = mydisk->fat1[page].id;
		//	memset(head,0,BLOCKSIZE);
		//}
		i=0;	//index = 0
		if(len>BLOCKSIZE)
		{
			count = len/BLOCKSIZE;
			offset = len % BLOCKSIZE;
			page = openfilelist[fd].first;
			//printf("coune%d offset%d",count,offset);		
			for(i=0;i<count;i++)
			{
				//printf("page number :%d\n",page);
				memcpy(buff,text+i*BLOCKSIZE,BLOCKSIZE);	//write into buffer
				memcpy(head,buff,BLOCKSIZE);	//write into virtual memory
				if(mydisk->fat1[page].id==END)	//
				{
					block = my_malloc(); //malloc area
					if(block != -1)
					{
						mydisk->fat1[page].id=block;
						page = block;
					}
					else 
						printf("virtual memory is full\n");
						//printf("malloc block num:%d\n",block);
				}
				else
				{
					page = mydisk->fat1[page].id;
				}
				head = mydisk->data[page];
			}
			templen = i*BLOCKSIZE;
			len = len - templen;
		}
		//printf("page number :%d\n",page);
		memset(buff,'\0',BLOCKSIZE); //flush buffer		
		memcpy(buff,text+i*BLOCKSIZE,len);	//copy to buffer 
		memcpy(head,buff,BLOCKSIZE);	//write to virtual memory
		openfilelist[fd].length = len+templen;	//update length
		openfilelist[fd].fcbstate = 1;	//file fcb was modified
	}
	if(mydisk->fat1[page].id != END) //free area
	{
		freepage = mydisk->fat1[page].id; //next page
		mydisk->fat1[page].id = END;
		for(page=freepage;mydisk->fat1[page].id != END;)
		{
			head = mydisk->data[page];
			memset(head,0,BLOCKSIZE);
			freepage = page;
			page = mydisk->fat1[page].id;
			mydisk->fat1[freepage].id = FREE;
		}
		head = mydisk->data[page];
		memset(head,0,BLOCKSIZE);
		mydisk->fat1[page].id = FREE;
	}
	return templen+len;
}



int my_read(char fd)
{
	int i;
	int len;
	char text[1024*10];
	i = fd-'0';
	//printf("%d",i);
	if(i>0 && i<MAXOPENFILE)
	{
		if(openfilelist[i].topenfile==1)
		{
			printf("input the read length :\t");
			scanf("%d",&len);
			while(( getchar())!='\n');	//clear buffer
			if(len<0)
			{
				printf("Invalid argument\n");
				return 0;
			}
			else if(len>openfilelist[i].length)
				len = openfilelist[i].length;
			//if(len==0)
			//	return 0;
			//printf("read len:%d",len);
			if((len = do_read(i,len,text))!=-1)
			{
				text[len] = '\0';
				//printf("\n%d\n",strlen(text));
				printf("############## %11s             ################\n",openfilelist[i].filename);
				printf("%s\n",text);
				printf("##############    %8d                 ################\n",len);
				return len;
			}
			else
				printf("error\n");
		}
		else
		{
			printf("don't hane this file\n");
		}
	}
	else
	{
		printf("Invalid argument\n");
		return -1;
	}
}

int do_read(int fd, unsigned long len, char *text)
{
	int fb;
	int page;
	int offset;
	char *head;
	char buff[BLOCKSIZE];
	int i;
	int count = 0;
	page = len/BLOCKSIZE;
	offset = len%BLOCKSIZE;
	fb = openfilelist[fd].first;
	//printf("read step 1\n");
	//printf("page number :%d\n",fb);
	for(i=0;i<page;i++)
	{
		//printf("page number :%d\n",fb);
		//printf("read step 2\n");
		head=mydisk->data[fb];
		memcpy(buff,head,BLOCKSIZE);
		//printf("%s\n\n",buff);
		memcpy(text+i*BLOCKSIZE,buff,BLOCKSIZE);
		fb=mydisk->fat1[fb].id;
		//printf("\n\n%d\n\n",i);
		count = i+1;
	}
	
//	printf("read step 3\n");
	if(offset!=0)
	{
		//printf("page number :%d\n",fb);
		head=mydisk->data[fb];
		//printf("read step 4\n");
		memcpy(buff,head,BLOCKSIZE);
		//printf("%s\n\n",buff);
		//printf("read step 5\n");
		//printf("\n\ncount%d\n\n",count);
		memcpy(text+count*BLOCKSIZE,buff,offset);
	}
	//printf("read step 6\n");
	return len;
}

int my_exitsys()
{
	int offset;
	for(offset=1;offset<MAXOPENFILE;offset++)	//checking the openfilelist
	{
		if(openfilelist[offset].topenfile!=0)
		{
			printf("some files were opened,please close it\n");
			return -1;
		}
	}
	FILE *f = fopen("my_fs","w");
	if(f==NULL)
		perror("virtaul memory error:");
	else
	{
		fwrite(myvhard,sizeof(char),SIZE,f);
		fclose(f);
	}
	free(myvhard);
	printf("exit file system\n");
	return 0;
}



void listopen()
{
	int offset;
	printf("\t------------------open file list--------------------\n");
	printf("\tfd\t rw_point\t length\t\t filepath\t\n");
	for(offset=0;offset<MAXOPENFILE;offset++)
	{
		if(openfilelist[offset].topenfile!=0)
		{
			printf("\t%d\t %d \t\t %ld \t\t %s\n",offset,openfilelist[offset].rw,openfilelist[offset].length,openfilelist[offset].dir);
		}
	}
	printf("\t------------------open file list--------------------\n");
}

void chmod(char *dirname)
{
	int right;
	int count = 0;
	int offset = 0;
	char tempdir[80]; //copy the currentdir
	char absdir[80]; 	//absoluted file path
	fcb *cfcb;
	fcb *pfcb;
	strcpy(absdir,currentdir);
	if(strlen(dirname)==0)		//no argument
	{
		printf("my_chmod:missing file path\n");
		return;
	}
	else if(strcmp(dirname,"./\0")==0)
	{
		printf("error \"./\": IS a directory\n");
		return;
	}
	else 
	{
		if(strcmp(dirname,"/\0") != 0 || (strcmp(absdir,"/\0")==0 && (strcmp(dirname,"./\0")==0 || strcmp(dirname,"../\0")==0 ))!=0 )
		{
			cfcb = search(dirname);
		//	printf("namename:%s\n",cfcb->filename);
			if(cfcb != NULL)
			{
				if(cfcb->attribute != 0)	//is file
				{
					strcpy(tempdir,absdir);
					if(dirname[0]=='/')	//absolute file path
					{
						while(*(dirname+offset) != '\0') //calculate the offset and parent file path
						{
							if(*(dirname+offset)=='/')
								count = 0;
							else
								count++;
							offset++;
						}
						*(dirname+offset-count) = '\0'; // parent file path
						pfcb = search(dirname);	//parent fcb	
						//printf("//////name:%s\n",pfcb->filename);
					}
					else	//relative file path
					{
						
						count = 0;
						offset = 0;
						if(dirname[0]=='.')  // file path "./..." or "../" or "../..."
						{
							//printf("type of ./.../\n");
							if(dirname[1] == '.')
							{
								while(*(tempdir+offset) != '\0')
								{
									if(*(tempdir+offset) == '/')
										count =0;
									else 
										count++;
									offset++;
								}
								tempdir[offset-count-1] = '\0';	//currentdir parent path
								if(strcmp(dirname,"../\0")!=0)
									strcat(tempdir,dirname+2);
								else 
									strcat(tempdir,dirname+3);	
							}
							else //file path "./...."
							{
								strcat(tempdir,dirname+1);
							}
						}
						else	//path type "...../"
						{
							if(strcmp(absdir,"/\0")!=0)
								strcat(tempdir,"/\0");
							strcat(tempdir,dirname);	//absolute path
							
						}
						strcpy(dirname,tempdir);	//operation file path
						//printf("\n\n%s\n\n",tempdir);
					}
					offset = 0;
					while(*(dirname+offset)!='\0')
					{
						offset++;
					}
					if(dirname[offset-1]=='/')
						dirname[offset-1]=='\0';	// delete the last '/'
					printf("input the right:");
					scanf("%d",&right);
					while(( getchar())!='\n');	//clear buffer
					strcpy(absdir,currentdir);
					for(count=1;count<MAXOPENFILE;count++)
					{
						if((openfilelist[count].dir,dirname)==0)
						{
							openfilelist[count].right=right;
							openfilelist[count].fcbstate=1;
							return;
						}
					}
					cfcb->right = right;
					return;
				}
				else
				{
					printf("chmod error\"%s\": is a directory\n",dirname);
				}
			}
			else
			{
				printf("chmod error \"%s\":no such dirctory\n",dirname);
			}
		}
		else
		{
			printf("chmod \"%s\": Is a dirctory\n",dirname);
		}
	}
	return;
}

void my_rename(char *dirname)
{
	char name[12];
	int count = 0;
	int offset = 0;
	char tempdir[80]; //copy the currentdir
	char absdir[80]; 	//absoluted file path
	fcb *cfcb;
	fcb *pfcb;
	strcpy(absdir,currentdir);
	if(strlen(dirname)==0)		//no argument
	{
		printf("rename:missing file path\n");
		return;
	}
	else if(strcmp(dirname,"/\0")==0)
	{
		printf("error \"/\": IS root directory\n");
		return;
	}
	else 
	{
		if( !(strcmp(absdir,"/\0")==0 && (strcmp(dirname,"./\0")==0 || strcmp(dirname,"../\0")==0) )  )
		{
			cfcb = search(dirname);
			//printf("namename:%s\n",cfcb->filename);
			if(cfcb != NULL)
			{
				strcpy(tempdir,absdir);
				if(dirname[0]=='/')	//absolute file path
				{
					while(*(dirname+offset) != '\0') //calculate the offset and parent file path
					{
						if(*(dirname+offset)=='/')
							count = 0;
						else
							count++;
						offset++;
					}
					*(dirname+offset-count) = '\0'; // parent file path
					pfcb = search(dirname);	//parent fcb	
					//printf("//////name:%s\n",pfcb->filename);
				}
				else	//relative file path
				{
					
					count = 0;
					offset = 0;
					if(dirname[0]=='.')  // file path "./..." or "../" or "../..."
					{
						//printf("type of ./.../\n");
						if(dirname[1] == '.')
						{
							while(*(tempdir+offset) != '\0')
							{
								if(*(tempdir+offset) == '/')
									count =0;
								else 
									count++;
								offset++;
							}
							tempdir[offset-count-1] = '\0';	//currentdir parent path
							if(strcmp(dirname,"../\0")!=0)
								strcat(tempdir,dirname+2);
							else 
								strcat(tempdir,dirname+3);	
						}
						else //file path "./...."
						{
							strcat(tempdir,dirname+1);
						}
					}
					else	//path type "...../"
					{
						if(strcmp(absdir,"/\0")!=0)
							strcat(tempdir,"/\0");
						strcat(tempdir,dirname);	//absolute path
						
					}
					strcpy(dirname,tempdir);	//operation file path
					//printf("\n\n%s\n\n",tempdir);
				}
				offset = 0;
				while(*(dirname+offset)!='\0')
				{
					offset++;
				}
				if(dirname[offset-1]=='/' && strcmp(dirname,"/\0")!=0)
					dirname[offset-1]=='\0';	// delete the last '/'
					
					
				printf("input the name:");
				scanf("%s",name);
				while(( getchar())!='\n');	//clear buffer
				//the modify path,if has exist retuen error
				strcpy(tempdir,dirname);
				count = 0;
				offset = 0;
				while(*(dirname+offset) != '\0')
				{
					if(*(dirname+offset) == '/')
						count =0;
					else 
						count++;
					offset++;
				}
				dirname[offset-count-1] = '\0';	//currentdir parent path
				if(dirname[offset-count-2]!='/' && strcmp(dirname,"/\0")!=0)
					strcat(dirname,"/\0");

					
				strcat(dirname,name);
				if(search(dirname)!=NULL)
				{
					printf("error \"%s\" file have exits\n",dirname);
					return;
				}

				for(count=1;count<MAXOPENFILE;count++)
				{
					if(strcmp(openfilelist[count].dir,tempdir)==0)
					{
						if(strcmp(tempdir,currentdir)==0)
						{
							strcat(currentdir,dirname);
						}
						memcpy(openfilelist[count].dir,dirname,80);	
						memcpy(openfilelist[count].filename,name,11);
						openfilelist[count].fcbstate=1;
						memcpy(cfcb->filename,name,11);
						return;
					}
				}
				if(strcmp(tempdir,currentdir)==0)
				{
					strcat(currentdir,dirname);
				}
				memcpy(cfcb->filename,name,11);
				return;
			}
			else
			{
				printf("renaem error \"%s\":no such dirctory\n",dirname);
			}
		}
		else
		{
			printf("rename \"/\": Is root dirctory\n");
		}
	}
	return;
}

void my_ls(char *dirname)
{
	int offset = 0;
	int count = 0;
	char dir[80];
	
	int i,j;
	unsigned short fb;		//file start block
	unsigned short pfb;		//parrent start block
	fcb *cfcb;		//currentfcb
	fcb *f;
	fcb *pf;	//parent fcb;
	fb = openfilelist[0].first;	//first block
	pfb = fb;	//parent first block
	pf = (fcb*)mydisk->data[fb];  
	memcpy(dir,currentdir,80);
	
	if(openfilelist[0].attribute==1)
		printf("It is not a directory\n");
	else 
	{
		cfcb = search(dirname);
		if(cfcb != NULL)
		{
			if(cfcb->attribute==0)		//is directory
			{
				if(strlen(dirname)!=0)	//path is not "\0" 
				{
					if(dirname[0]=='.')	// file path "./....."
					{
						if(dirname[1]=='.')  // file path "../....."
						{
							while(*(dir+offset) != '\0')
							{
								if(*(dir+offset)=='/')
									count = 0;
								else 
									count++;
								offset++;
							}
							dir[offset-count-1] = '\0';		//parent path
							//printf("\n%s\n%d\n%d",currentdir,count,offset);
							if(strcmp(dirname,"../\0")!=0)
								strcat(dir,dirname+2);

						}
						else
							strcat(dir,dirname+2);
					}
					else if(dirname[0]=='/')
					{
						strcpy(dir,dirname);
					}
					else
					{
						if(strcmp(dir,"/\0") != 0)
							strcat(dir,"/\0");
						strcat(dir,dirname);
					}
				}
				else 	//
				{
					cfcb = search(dir);
				}
				
				
				printf("filename\tattribute\ttime  \t\tdate\t\tlength\tright\n");
				
				f = (fcb*)mydisk->data[cfcb->first];
				if(f->length < BLOCKSIZE)	//search current block
				{
					for(i=0; i< (int)(f->length/sizeof(fcb));i++)
					{
						printf("%-11s \t%d \t\t%02d:%02d:%02d\t%4d-%02d-%02d \t%ld \t%d\n",(f+i)->filename,(f+i)->attribute,(f+i)->time>>11,((f+i)->time&2016)>>5,(f+i)->time&31,((f+i)->date>>9)+1980,((f+i)->date&480)>>5,(f+i)->date&31,(f+i)->length,(f+i)->right);
					}
				}
				else	//serach many blocks
				{
					for(i=0;i<(int)(f->length/BLOCKSIZE);i++)
					{
						for(j=0; j < (BLOCKSIZE/sizeof(fcb));j++) //search  blocks
						{
							printf("%-11s \t%d \t\t%02d:%02d:%02d\t%4d-%02d-%02d \t%ld \t%d\n",(f+j)->filename,(f+j)->attribute,(f+j)->time>>11,((f+j)->time&2016)>>5,(f+j)->time&31,((f+j)->date>>9)+1980,((f+j)->date&480)>>5,((f+j)->date & 31),(f+j)->length,(f+j)->right);
						}
						fb = mydisk->fat1[fb].id;
						f = (fcb*)mydisk->data[fb];
					}

					for(i=0; i< (int)((f->length%BLOCKSIZE)/sizeof(fcb));i++)  //last block
					{
						printf("%-11s \t%d \t\t%02d:%02d:%02d\t%4d-%02d-%02d \t%ld \t%d\n",(f+i)->filename,(f+i)->attribute,(f+i)->time>>11,((f+i)->time&2016)>>5,(f+i)->time&31,((f+i)->date>>9)+1980,((f+i)->date&480)>>5,(f+i)->date&31,(f+i)->length,(f+i)->right);
					}
				}
			}
			else 
				printf("\"%s\":Not a directory\n",dirname);
			
		}
		else 
		{
			printf("%s:no such file or dircetory5",dirname);
		}
	}
}

void locate_rw(char fd)
{
	int i = fd - '0';
	
}

void setdate(fcb *f)
{
	time_t timep;
	struct tm *p;
	time (&timep);
	p=localtime(&timep);
	f->date = 0;
	f->time =0;
	f->date ^= (~0&(p->tm_year-80))<<9;		// p->tm_year offset off 1900; 
	f->date ^= (~0&p->tm_mon+1)<<5;
	f->date ^= (~0&p->tm_mday);
	f->time ^= (~0&p->tm_hour)<<11;
	f->time ^= (~0&p->tm_min)<<5;
	f->time ^= (~0&p->tm_sec);
}
