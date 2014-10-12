#include "rvm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
int numsegs = 0;
int entered=0;
int initflag = 0;
rvm_t rvmid;
int counter=0;
int size_to_write=0;
int first=0;
struct segments
{
rvm_t rvm;
int being_modified;
trans_t tid;
int mapped;
int considered;
char *segname;
char *segment;
char *storename;
int segsize[10];
int trans_begun;
int offsetcount;
int tidcount;
long offsets[10];
struct segments *next;
};
const char *check;
int head =1,tail=1;
int number_of_direcs=0;
struct segments *segments,*headnode;

FILE *logfile, *seglog,*newfile;
char logpath[200];

int segindex=0;
FILE *backstore;
int result;
//long offs;
//long offsets[10000];
int offsetcount=0;
FILE *offcount;
//char *line;
rvm_t rvm_init(const char *directory) // Initialize the library with the specified directory as backing store.
{	int i=0;
	rvm_t rvm;
	char *filepath, *backpath, *seglogpath, *t,*pathlog;
char storename[200];
	struct stat file1;
	char temp[200];
	
	strcpy(temp,"mkdir ");
	t = (char*)malloc(200*sizeof(char));
	filepath = (char*)malloc(200*sizeof(char));
	strcpy(t,strcat(temp, directory));
	
	strcpy(rvm.dir,directory);
	//printf("rvm dir is %s\n",rvm.dir);
	int status = stat(directory, &file1);

	if(status == -1)
	system(t);
	strcpy(temp,directory);
	//printf("temp is %s\n",temp);
	filepath = strcat(temp, "/logfile");
	strcpy(logpath,filepath);	
	
	entered = 1;
	number_of_direcs++;
	//printf("rvm id after assign is %d\n",rvm_array[0].rvmid);
	
	return rvm;

}


void *rvm_map(rvm_t rvm, const char *segname, int size_to_create) // map a segment from disk into memory. If the segment does not already exist, then create it and give it size size_to_create. If the segment exists but is shorter than size_to_create, then extend it until it is long enough. It is an error to try to map the same segment twice.
{
int allocated=0;
if(rvm.dir==NULL)
{
printf("dirt is NULL\n");
	exit(-1);
}
//printf("segment nsame passed %s\n",segname);
struct segments *temp_seg = (struct segments *)malloc(sizeof(struct segments));
struct segments *node = (struct segments *)malloc(sizeof(struct segments));
//struct segments *segments;

char *filepath, *backpath, *seglogpath, *t,*pathlog;
char storename[200];

	struct stat file1;
	//char *save = (char*)malloc(sizeof(char)*100000);
	char line[10000];
char save[100000];
//printf("INMAP\n");
	char temp[200];int z;
	
	allocated=0;
	strcpy(storename,rvm.dir);
	backpath = (char*)malloc(200*sizeof(char));
	
	strcat(storename,"/");
       
	strcat(storename,segname);
	int status = stat(storename,&file1);
	
	int len=0;
	

	
	if(first==0){

	if(status == -1){
	//printf("opening backpath %s\n",storename);
	
		}
	else{
	
	backstore =fopen(storename,"r");
	
	
	while(fgets(line,10000,backstore)!=NULL){
		len = strlen(line);
		strcat(save,line);
		}
	
	}

	segments = (struct segments *)malloc(sizeof(struct segments));
        segments->segment = (char*)malloc(sizeof(char)*size_to_create);
	segments->storename = (char*)malloc(sizeof(char)*100);
	segments->segname=(char*)malloc(sizeof(char)*100);
	segments->offsetcount = offsetcount;
	strcpy(segments->storename,segname);
	//segments->uncommitted = 1;
	strcpy(segments->segname,segname);
	segments->mapped = 1;
	segments->rvm = rvm;
	segments->next = NULL;
	headnode = segments;
	first=1;
		if(status == 0){
		int d=0;
		for(d=0; d<temp_seg->offsetcount;d++)
		temp_seg->offsets[temp_seg->offsetcount] = 0;
			temp_seg->offsetcount = 0;
		int index =0,counter=0;
		offsetcount=0;
		char ch = save[index];
		while(counter < len)
		{
		while(save[counter] == '$' && counter < len)
			counter++;
		if(save[counter]!='$')
			segments->offsets[segments->offsetcount++] = counter++;
		while(save[counter]!='$' && counter < len)
			counter++;
		}		

	
		for(z=0; z<segments->offsetcount; z++){
		char arr[200];int c=0,p=0;
		char temp = save[c];

		while(temp=='$'){
			temp = save[++c];
			//c++;
			}
		while(temp != '$')
		{
		arr[p]=temp;
		c++;
		p++;
		temp = save[c];
		}	
		arr[p]='\0';
		
		strcpy(segments->segment+segments->offsets[z],arr);
		}		
	offsetcount=0;
	//free(save);
	fclose(backstore);
	fflush(backstore);
	}	

	return segments->segment;

	}




	else
	{


	if(status == -1){
	//printf("opening backpath %s\n",storename);
	
		}
	else{
	
	backstore =fopen(storename,"r");
	
	
	while(fgets(line,10000,backstore)!=NULL){
		len = strlen(line);
		strcat(save,line);
		}
	
	}

	segments = headnode;
	while(segments!=NULL)
	{
	if(!strcmp(segments->segname,segname)){
		allocated = 1;
		segments->segment = realloc(segments->segment,sizeof(char)*size_to_create);
		return segments->segment;
		//break; 	
			}
		if(segments->next == NULL)
			break;
		segments = segments->next;
		}

	temp_seg->segment =(char*)malloc(sizeof(char)*size_to_create);
	temp_seg->storename = (char*)malloc(sizeof(char)*100);
	temp_seg->rvm = rvm;
	//temp_seg->uncommitted = 1;
	temp_seg->segname=(char*)malloc(sizeof(char)*100);
	strcpy(temp_seg->storename,segname);
	
	//strcpy(temp_seg->seglogname,segname);
	strcpy(temp_seg->segname,segname);
	temp_seg->mapped = 1;
	temp_seg->next = NULL;
	
	
	//Write the rvm id
	if(status == 0){	
	int index =0,counter=0,d=0;
	offsetcount = 0;
	char ch = save[index];
			for(d=0; d<temp_seg->offsetcount;d++)
		temp_seg->offsets[temp_seg->offsetcount] = 0;
	temp_seg->offsetcount = 0;

		while(counter < len)
		{
		while(save[counter] == '$' && counter < len)
			counter++;
		if(save[counter]!='$')
			temp_seg->offsets[temp_seg->offsetcount++] = counter++;
		while(save[counter]!='$' && counter < len)
			counter++;
		}	



		for(z=0; z<temp_seg->offsetcount; z++){
		char arr1[20000];int c=0,p=0;
		char temp = save[c];
		
		while(temp=='$'){
			temp = save[++c];
			//c++;
			}
		while(temp != '$')
		{
		
		arr1[p]=temp;
		c++;
		p++;
		temp = save[c];
		}	
		arr1[p]='\0';
		
		strcpy(temp_seg->segment+temp_seg->offsets[z],arr1);
				
		}		
	offsetcount=0;
	//free(save);
	fclose(backstore);
	fflush(backstore);
	}	
	
	
	segments->next = temp_seg;
	segments = segments->next;
	//headnode = segments;
	
	return segments->segment;
	}
	



}

void rvm_unmap(rvm_t rvm, void *segbase) // unmap a segment from memory.
{

	if(headnode==NULL)
		{
			//printf("In unmap and headnoce NULL\n");
		exit(-1);}
	int i,found=0,m;	
	char temp[200];
	int ofset;
	head =1;
	int node=1;
	struct segments *temp_seg = (struct segments *)malloc(sizeof(struct segments));
	struct segments *remove = (struct segments*)malloc(sizeof(struct segments));
	temp_seg = headnode;
	if(temp_seg==NULL)
		exit(0);	
	
	while(temp_seg!=NULL){
		m=0;
		temp_seg->tidcount += 1;
		if(temp_seg->offsetcount < 1)
			ofset = 1;
		else
			ofset = temp_seg->offsetcount;
	
			for(m=0;m<ofset;m++)
		{
		if(!strcmp(temp_seg->segment+temp_seg->offsets[m],segbase)){
			found=1;
			temp_seg->mapped=0;
			
									
										//printf("REMOVING");
										if(node){
										//printf("removed\n");
										if(temp_seg->next == NULL)
											temp_seg=NULL;
										else
										temp_seg = temp_seg->next;
										node=0;}
										//temp_seg->mapped = 0;
										//temp_seg->being_modified = 0;}
										else
										{
										remove->next = temp_seg->next;
										temp_seg = remove;
										}
									//remove = remove;
									//remove=remove->next;
			//free(temp_seg->segment);
			//free(temp_seg->segname);
			break;
			}
		}
		remove = temp_seg;
		head = 0;
		remove = temp_seg;
			if(found==1)
			break;
		temp_seg = temp_seg->next;}
		//headnode = NULL;
			if(temp_seg==NULL)
				first = 0;
		while(temp_seg!=NULL)
			exit(0);
		
	if(found==0){
			printf("not found segment in unmap\n");
		exit(-1);}
}


void rvm_destroy(rvm_t rvm, const char *segname) //destroy a segment completely, erasing its backing store. This function should not be called on a segment that is currently mapped.
{
struct stat file2;


char *filepath, *backpath, *seglogpath, *t,*pathlog;
char *storename;

        int head =1;char temp[200],found=0;
        struct segments *temp_seg = (struct segments *)malloc(sizeof(struct segments));
        struct segments *remove = (struct segments*)malloc(sizeof(struct segments));
        temp_seg = headnode;
	int status = stat(segname,&file2);
	
	if(temp_seg==NULL)
		return;
        while(temp_seg!=NULL){
		
                if(!strcmp(temp_seg->segname,segname)){
                        found=1;
			if(!temp_seg->mapped){
                        if(head)
                        segments = segments->next;
                        else{
                        remove->next = temp_seg->next;
                        free(temp_seg);
                        }
			
                        strcpy(temp,"rm ");
                        t = strcat(temp,rvm.dir);
                        t = strcat(temp,"/");
                        t = strcat(temp,segname);
                        system(t);
			break;
				}
			
			else exit(-1);}
                remove = temp_seg;
                head = 0;
                temp_seg = temp_seg->next;}
        if(found==0)
                exit(-1);

}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases) // begin a transaction that will modify the segments listed in segbases. If any of the specified segments is already being modified by a transaction, then the call should fail and return (trans_t) -1.
{
	tail = 1;
	int node=1;
	char *filepath, *backpath, *seglogpath, *t,*pathlog;
	char *storename;
	int tid,fst,match=0;
	struct stat first;
	//printf("in trans\n");
	int k=0;char temp[200];
        struct segments *temp_seg = (struct segments *)malloc(sizeof(struct segments));
        struct segments *remove = (struct segments*)malloc(sizeof(struct segments));
	 struct segments *temp_remove = (struct segments *)malloc(sizeof(struct segments));
	remove = headnode;
	/*printf("coming hre\n");
								while(remove!=NULL){
									
									if(temp_seg->tidcount > 0){
										printf("REMOVING");
										if(node){
										remove = remove->next;
										node=0;
										remove->mapped = 0;
										remove->being_modified = 0;}
										else
										{
										temp_remove->next = remove->next;
										remove = temp_remove;
										}
									}
									temp_remove = remove;
									remove=remove->next;
									}
	remove = headnode;
	while(remove!=NULL){
		printf("should only be one\n");
		//printf("value in segment is name %s\n",temp_seg->segname);
		remove = remove->next;
			if(remove->next == NULL)
				break;
		}*/
	temp_seg = headnode;
	int m,ofset=0,count=0;
	while(k<numsegs)
		{	temp_seg = headnode;
		match = 0;
		while(temp_seg!=NULL)
			{
			int head=1;
			//printf("My offsets are %d\n",temp_seg->offsetcount);
			if(temp_seg->offsetcount == 0)
				ofset = 1;
			else
				ofset = temp_seg->offsetcount;
			if(k<=numsegs)
			for(m=0;m<ofset;m++)
				{
				//printf("segbase is and temp seg is %s %s \n",segbases[k],temp_seg->segment+temp_seg->offsets[m]);
				if(!strcmp(temp_seg->segment+temp_seg->offsets[m],(char*)segbases[k]+temp_seg->offsets[m]) && temp_seg->tidcount ==0)
					{
					
					//k++;
					/*if(temp_seg->being_modified == 1)
						{

						return (temp_seg->tid -1);}*/
						if(temp_seg->considered==0)
							
							{
							match = 1;
							if(tail)
								{ 
								tid = rand();
								//remove = temp_seg;
								// = headnode;
								temp_seg->tid = tid;
								//temp_seg->being_modified = 1;
								temp_seg->considered = 1;
								//printf("trans for %s\n",temp_seg->segname);
								tail=0;
								count++;
								fst = stat(logpath,&first);
								if(fst==-1)
								logfile = fopen(logpath,"w");
								else logfile = fopen(logpath,"a");
								if(fst == -1)
								{
								fprintf(logfile,"$");
								fprintf(logfile,"%d_",tid);
								fprintf(logfile,"n_");
								}
								//fprintf(logfile,temp_seg->segname);
								fclose(logfile);
								fflush(logfile);
								//printf("Closed logfile");
\
								}
							else
								{
								//printf("adding the secondtransaction to header\n\n");
								temp_seg->tid = tid;
								count++;
								temp_seg->considered = 1;
								//printf("count is %d\n",count);
								//printf("segname is %s\n",temp_seg->segname);
								fst = stat(logpath,&first);
								if(fst==-1)
								logfile = fopen(logpath,"w");
								else logfile = fopen(logpath,"a");
								if(fst == -1){
									fprintf(logfile,"$");
									fprintf(logfile,"%d_",tid);
									fprintf(logfile,"n_");
									}
								//fprintf(logfile,temp_seg->segname);
								fclose(logfile);
								fflush(logfile);
								//printf("Closed logfile");
								//remove = temp_remove;

								} // close of else for modified == 1
							} //close of else modified			;

						} // strcmp
					if(match == 1)
						break;
					}//close of for
				if(match == 1)
					break;
				if(temp_seg->next==NULL)
					{
						break;
					}
				//printf("advancing pointer\n");
				temp_seg = temp_seg->next;
			}//close of while
//temp_seg = temp_seg->next;
		k++;
	}//close of while
temp_seg = headnode;
while(temp_seg!=NULL){
//printf("modified\n");
//printf("all tids in the array are %d\n",temp_seg->tid);
if(temp_seg->tid!=tid){
	temp_seg->tidcount += 1;
	}
if(temp_seg->considered == 1)
{
temp_seg->being_modified = 1;
temp_seg->mapped = 1;
temp_seg->considered = 0;
//temp_seg->uncommitted = 1;
//remove->tid = tid;
}
temp_seg= temp_seg->next;
}
return tid;
}




void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size) //declare that the library is about to modify a specified range of memory in the specified segment. The segment must be one of the segments specified in the call to rvm_begin_trans. Your library needs to ensure that the old memory has been saved, in case an abort is executed. It is legal call rvm_about_to_modify multiple times on the same memory area.
{
char *filepath, *backpath, *seglogpath, *t,*pathlog;
char storename[200];

//need to use fseek to write memory within that range as specified.open log and back store with same range.but herer i need to note them down not write them to the backing store...

int i=0;char temp[200],match=0,ofset=0;

        struct segments *temp_seg = (struct segments *)malloc(sizeof(struct segments));
        struct segments *remove = (struct segments*)malloc(sizeof(struct segments));
         struct segments *temp_remove = (struct segments *)malloc(sizeof(struct segments));
temp_seg = headnode;
int z,m;

//offsets[offsetcount++] = (long) offset;
while(temp_seg!=NULL)
	{
	m=0;
	//printf("data in the segment %s\n\n",temp_seg->segment);
	//printf("in the segments list\n\n");
	if(temp_seg->offsetcount > 0)
	{
	ofset = temp_seg->offsetcount;
	}
	else
	ofset = 1;
	//compare = strcmp(temp_seg->segment+temp_seg->offsets[ofset],segbase);
	for(m=0;m<ofset;m++)
	{
	if(!strcmp(temp_seg->segment+temp_seg->offsets[m],segbase) && temp_seg->tidcount==0)
	{
char *line;
//temp_seg->tid = tid;
if(temp_seg->tid == tid)// && temp_seg->trans_begun!=1)
{
//printf("offset count is %d\n",temp_seg->offsetcount);
match = 1;

//printf("found the segment with transaction\n");
pathlog = (char*)malloc(200*sizeof(char));

int h,duplicate=0,position=0;
for(h=0;h<temp_seg->offsetcount;h++)
{
if(temp_seg->offsets[h] == (long)offset)
{
position = h;
duplicate = 1;
}
}
if(duplicate == 0)
{
//printf("offset values inside are %ld and offset is %ld\n",temp_seg->offsets[temp_seg->offsetcount],offset);
temp_seg->offsets[temp_seg->offsetcount] = (long) offset;
temp_seg->offsetcount++;
//printf("canot come here\n");
}
else
temp_seg->offsets[position] = (long) offset;


//printf("offset count of the segmens %s are %d\n\n",temp_seg->segname,temp_seg->offsetcount);
logfile = fopen(logpath,"a");
fprintf(logfile,"@");
for(z=0;z<temp_seg->offsetcount;z++)
{
fprintf(logfile,"%s",temp_seg->segname);
fprintf(logfile,"_%d_",temp_seg->offsets[z]);
fprintf(logfile,temp_seg->segment+temp_seg->offsets[z]);
fprintf(logfile,"@");
//fprintf(logfile,"%");
}
fclose(logfile);

//printf("\nOffsetcount for segments in order are %d\n",temp_seg->offsetcount);

strcpy(storename,temp_seg->rvm.dir);
temp_seg->considered = 0;
//pathlog = strcat(pathlog,temp_seg->seglogname);
strcat(storename,"/");

strcat(storename,temp_seg->storename);
//FILE *f = fopen(pathlog,"rw");
//printf("STORENAME=%s\n",storename);

size_to_write = size;



break;
	}

}
if(match == 1)
	break;
}
if(match == 1)
	break;
temp_seg = temp_seg->next;
	}

}


void rvm_commit_trans(trans_t tid) //commit all changes that have been made within the specified transaction. When the call returns, then enough information should have been saved to disk so that, even if the program crashes, the changes will be seen by the program when it restarts.
{
char *filepath, *backpath, *seglogpath, *t,*pathlog;
FILE *f2;
struct stat file2;
//printf("commit\n");
//need to use fseek to write memory within that range as specified.open log and back store with same range.but herer i need to note them down not write them to the backing store...

char temp[200];
        struct segments *temp_seg = (struct segments *)malloc(sizeof(struct segments));
temp_seg = headnode;
int p=0;

while(temp_seg!=NULL)
{
//printf("NO of nodes in the list are %d\n",p);
p++;
temp_seg = temp_seg->next;
}

temp_seg = headnode;
FILE *f, *f3;
int i;

//if(temp_seg == NULL)
//	printf("NOT POSSIBLE\n\n");

while(temp_seg!=NULL)
{
char storename[100];
//printf("tids are as follows %d\n\n\n",temp_seg->tid);
//printf("passes tid is %d and value of modifierd is %d\n",tid,temp_seg->being_modified);
//printf("tid count is %d and tid pases is %d\n",temp_seg->tid,tid);
//printf("value of segment here is %s\n",temp_seg->segment);
if(temp_seg->tid == tid && temp_seg->being_modified==1 && temp_seg->tidcount==0)// && temp_seg->mapped == 1)
{
//printf("in commit and segments are %s \n\n",temp_seg->segment);
//printf("in commit in segment list \n\n");

pathlog = (char*)malloc(200*sizeof(char));
//storename = (char*)malloc(200*sizeof(char));
strcpy(storename,temp_seg->rvm.dir);
//pathlog = strcat(pathlog,temp_seg->seglogname);
strcat(storename,"/");
strcat(storename,temp_seg->storename);
//printf("storenamse is %s\n\n",storename);
//printf("the segment who is about to be commited is %s and tid count is %d and offsetcount is %d\n",temp_seg->segment,temp_seg->tidcount,temp_seg->offsetcount);
temp_seg->being_modified = 0;
//temp_seg->uncommitted = 0;
int status2 = stat(storename,&file2);
//printf("status %d\n",status2);
if(status2 == -1){

//printf("STATUS -1\n");
f2 = fopen(storename,"w");

int max=0,temp_count=0,put_zeros=0,z=0,ofset=0;

//printf("\nIn commit anout to loop with offsetcount %d\n",temp_seg->offsetcount);
for(z=0; z<temp_seg->offsetcount; z++){
if(temp_seg->offsets[z] > max)
{
max = temp_seg->offsets[z];
} 
}
put_zeros = max;
//printf("the value of put zeros is %d\n",put_zeros);
if(put_zeros !=0)
{
while(put_zeros>=0)
{
fprintf(f2,"$");
put_zeros--;
}

}

rewind(f2);
//printf("postion is %ld\n",ftell(f2));

	if(temp_seg->offsetcount > 0)
	{
	ofset = temp_seg->offsetcount;
	}
	else
	ofset = 1;
 
for(z=0;z<temp_seg->offsetcount;z++)
{

fseek(f2,temp_seg->offsets[z],SEEK_SET);
//printf("value being commited is %s and %s\n",temp_seg->segment,temp_seg->segment+temp_seg->offsets[z]);
fprintf(f2,temp_seg->segment+temp_seg->offsets[z]);
}
//printf("IN commit and temp value is %s\n",temp_seg->segment);
fclose(f2);
fflush(f2);
}

else
{

char *prin = (char*)malloc(sizeof(char)*10000); ;
int len=0;
char line[1000];
//printf("STORENAME IS %s\n",storename);
f2 = fopen(storename,"r");
fseek(f2,0,SEEK_END);
long value = ftell(f2);
fseek(f2,0,SEEK_SET);

	while(fgets(line,1000,f2)!=NULL){
		len = strlen(line);
		strcat(prin,line);
		}


fclose(f2);
fflush(f2);

//printf("length is %d \n",len);
//printf("reading and writing %s\n\n",prin);
f2 = fopen(storename,"w");
if(len !=0)
fprintf(f2,prin);



//printf("STORENAME2 %s",storename);
//char temp2[100000];

//printf("value to be written is %s",temp2);
int z;

	
int max=0,temp_count=0,put_zeros=0;
int temp_array[offsetcount+1];
//printf("\nIn commit anout to loop with offsetcount %d\n",temp_seg->offsetcount);
for(z=0; z<temp_seg->offsetcount; z++){
if(temp_seg->offsets[z] > max)
{
max = temp_seg->offsets[z];
}
}
put_zeros = max;
//printf("the value of put zeros is %d\n",put_zeros);
if(max > len)
{
	while(put_zeros>=max)
	{
	fprintf(f2,"$");
	put_zeros--;
	}

}

rewind(f2);
//printf("postion is %ld\n",ftell(f2));
 
for(z=0;z<temp_seg->offsetcount;z++)
{

fseek(f2,temp_seg->offsets[z],SEEK_SET);
//printf("offset is %d\n",temp_seg->offsets[z]);
//printf("value being commited is %s and %s\n",temp_seg->segment,temp_seg->segment+temp_seg->offsets[z]);
fprintf(f2,temp_seg->segment+temp_seg->offsets[z]);
}

temp_seg->being_modified = 0;

//fseek(f2,0,SEEK_END);
/*long value = ftell(f2);
fseek(f2,0,SEEK_SET);*/
fclose(f2);
fflush(f2);
char *prin1 = (char*)malloc(sizeof(char)*10000);
char line1[1000];
int len1;
//printf("STORENAME IS %s\n",storename);
f2 = fopen(storename,"r");
fseek(f2,0,SEEK_END);
long value1 = ftell(f2);
fseek(f2,0,SEEK_SET);

	while(fgets(line1,1000,f2)!=NULL){
		len1 = strlen(line1);
		strcat(prin1,line1);
		}

//printf("value in file is %s\n",line1);
fclose(f2);

}
//LOG
long trk; char read[10000];
long pos1;
char ch;
int t;
f2 = fopen(logpath,"r");
fgets(read,10000,f2);
rewind(f2);
pos1 = strlen(read);
int l=0;int positions[10];

for(trk=0; trk<pos1; trk++)
{
ch = fgetc(f2);
++trk;

	if(ch == '$')
	 {
		while(ch != '_'){
		ch = fgetc(f2);
		++trk;
		
		}
		positions[l]= trk;
		l++;
	 }
}

fclose(f2);
fflush(f2);
f2=fopen(logpath,"w");
for(t=0;t<l;t++)
{
read[positions[t]] = 'c';
}
fprintf(f2,"%s",read);
fclose(f2);
fflush(f2);

//LOG ENDS
}
temp_seg=temp_seg->next;
}

}

void rvm_abort_trans(trans_t tid) //undo all changes that have happened within the specified transaction.
{
char *filepath, *backpath, *seglogpath, *t,*pathlog;
//printf("abort\n");
struct stat file1;
//erase stuff from the log from the specified offsets but only if not commited.
int i;char temp[200];
        struct segments *temp_seg = (struct segments *)malloc(sizeof(struct segments));
         struct segments *temp_remove = (struct segments *)malloc(sizeof(struct segments));
temp_seg = headnode;
char line[10000];
//printf("PASSED TID IS %d\n",tid);
while(temp_seg!=NULL)
{
//printf("stored tid is %d\n",temp_seg->tid);
char storename[1000];
if(temp_seg->tid == tid)
{
//printf("IT Has been MODIFIED\n\n");

temp_seg->being_modified = 0;
//temp_seg->mapped = 0;
//temp_seg->uncommitted = 0;
//pathlog = (char*)malloc(200*sizeof(char));
//storename = (char*)malloc(200*sizeof(char));
//printf("directory is %s\n",temp_seg->rvm.dir);
strcpy(storename,temp_seg->rvm.dir);
strcat(storename,"/");
//printf("%s\n",pathlog);
//pathlog = strcat(pathlog,temp_seg->segname);
strcat(storename,temp_seg->storename);
//printf("storename%s\n",storename);
    int fileSize = 0;
    int n, j;
    //printf("Opening file: \"%s\"\n", loc);
     
    FILE *pFile = fopen(storename, "r");
    if(pFile == NULL){
    exit(1);
    }
     
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    rewind(pFile);
     
    char *data = (char*) calloc(sizeof(char), fileSize + 20);
     
    fread(data, 1, fileSize, pFile);
    if(ferror(pFile)){
    exit(1);
    }
  
    fclose(pFile);
	fflush(pFile);
	int d=0;
    char save[10000];int len=0;offsetcount=0;
  //  printf("Done reading, parsing file now.\n");
char line[10000];

	//temp_seg->offsets = ;
	strcpy(save,data);
	int index =0,counter=0,z=0;
	//printf("value of oneis %c\n",save[1]);
	//printf("save is in abort%s\n",save);
	//fseek(backstore,0,SEEK_SET);
	//printf("IN ELSE\n");
	offsetcount = 0;
	char ch = save[index];

		while(counter < len)
		{
		while(save[counter] == '$' && counter < len)
			counter++;
		if(save[counter]!='$')
			temp_seg->offsets[temp_seg->offsetcount++] = counter++;
		while(save[counter]!='$' && counter < len)
			counter++;
		}	
	int h =0,ofset;
	//printf("came her in abort with offsetcount %d\n",temp_seg->offsetcount);
	if(temp_seg->offsetcount > 0)
	{
	ofset = temp_seg->offsetcount;
	}
	else
	ofset = 1;
int status = stat(storename,&file1);
	if(status == 0){

		for(z=0; z<ofset; z++){
		char arr[20000];int c=0,p=0;
		char temp = save[c];

		while(temp=='$'){
			temp = save[++c];
			//c++;
			}

		while(temp != '$')
		{
		arr[p]=temp;
		c++;
		p++;
		temp = save[c];
		}	
		arr[p]='\0';
		//printf("Mapping back existing segment\n");
		//printf("array is in abort %s\n",arr);
		strcpy(temp_seg->segment+temp_seg->offsets[z],arr);
		//printf("String is %s at offset %ld\n",temp_seg->segment+temp_seg->offsets[z], temp_seg->offsets[z]);		
		}		
	}	
//strcpy(temp_seg->segment,data);
}
temp_seg = temp_seg->next;
}

}



void rvm_truncate_log(rvm_t rvm) //play through any committed or aborted items in the log file(s) and shrink the log file(s) as much as possible.
{
//printf("YES");
char *filepath, *backpath, *seglogpath, *pathlog;
struct stat file1;
char temp[200];
int start[10],end[10];
strcpy(temp,rvm.dir);
strcat(temp,"/");
strcat(temp,"logfile");
int status = stat(temp,&file1);
int p=0,t=0,delete;
if(status == -1)
	return;

FILE *fp;
char ch1=' ';
int found;
char line[10000];char newstring[10000];char read[10000];
//printf("etemp is %s\n",temp);
fp = fopen(temp,"r");
fseek(fp,0,SEEK_END);
long pos = ftell(fp);
//printf("pos is%ld\n",pos);
rewind(fp);
	while(fgets(read,100000,fp)!=NULL){
		//len += strlen(reed);
		strcat(line,read);
		}
//printf("line is %s\n",line);
fclose(fp);
fflush(fp);
int i =0;
while(line[i]!='\0' || i<pos)
{
char ch = line[i];
if(ch == '$')
	{
	start[t] = i;
	i++;
	while(line[i]!='$' && line[i]!='\0' && i<pos)
		{
		int check = i;
		while(line[i]!='_' && line[check + 1]!='\0' && i<pos)
			{			
			ch1 = line[i++];
			if(ch1 == 'n'){
			found =1;
			delete = 1;}
			}
		i++;
		}
	if(delete == 1)
	{
	
	end[t] = i-1;
	t++;
	i = i - 1;
	delete = 0;
	}

	}
if(line[i] == '\0' || line[++i]=='\0')
	break;
i++;
}
if(found ==1)
{
for (p=0;p<t;p++)
{

strncpy(newstring,line+start[p],end[p]);
}
}
if(found!=1)
	strcpy(newstring,"");
fp = fopen(temp,"w");
fprintf(fp,newstring);
fclose(fp);
fflush(fp);
}
