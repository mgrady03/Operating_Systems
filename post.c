#define FUSE_USE_VERSION 26

#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>

#include "cs1550.h"


/* references used 

(void) before varibales to stop error about unused varibles 
https://stackoverflow.com/questions/7354786/what-does-void-variable-name-do-at-the-beginning-of-a-c-function

I was getting an error for truncate 
https://stackoverflow.com/questions/50198319/gcc-8-wstringop-truncation-what-is-the-good-practice

fseek function i needed help with 
https://www.geeksforgeeks.org/fseek-in-c-with-example/

this one is fread 
https://www.geeksforgeeks.org/fseek-in-c-with-example/

from that the rest fwrite was the same set up 







//1 sub 
//2 path 
//3 filename

f seek  is used to move the 
file pointer associated with a given file to a specific position.
The C fread() is a standard library function used to read 
the given amount of data from a file stream
fwrite is a function that writes to a FILE*, 
which is a (possibly) buffered stdio stream
*/
/*FUSE is a Linux kernel extension that allows for a userspace program 
to provide the implementations for the various file-related syscalls. 

README
*/

//ignore these helper method names i was losing my mind 



static int start_BLOCKed(char dname[]);//helper to get the first block
static int get_pathed_b(const char *path);//helper to check the path
//* type cs1550 type--readme helpers as well
static struct cs1550_directory_entry * file_entried_into_this_p(char name[]);//helper to find directory entry
static struct cs1550_file_entry * where_dat_file_at(struct cs1550_directory_entry *, char fname[], char fext[]);//helper

FILE *file; //disk file 
struct cs1550_root_directory *root;//root 
/**
 * Called whenever the system wants to know the file attributes, including
 * simply whether the file exists or not.
 *
 * `man 2 stat` will show the fields of a `struct stat` structure.
 */


/*
Since the disk contains blocks that are directories and blocks that are 
file data, we need to be able to find and identify what a particular block represents

README

*/

/*
This function should look up the input path to determine if it is a directory or a file.
If it is a directory, return the appropriate permissions. If it is a file, 
return the appropriate permissions as well as the actual size. 
This size must be accurate since it is used to determine EOF and thus read may not be called.
*/
static int cs1550_getattr(const char *path, struct stat *statbuf)
{
	// Clear out `statbuf` first -- this function initializes it.
	memset(statbuf, 0, sizeof(struct stat));//given 

	/* 
	setup:
 	check path is a valid path using helper methods created for ease of use 
  	get pathed---checks for path accuracy

   	next check if the math is in the root directory--given 

	 */
	if(get_pathed_b(path)==0)//false
		return -ENAMETOOLONG;//path is not valid so remove and exit 
	// Check if the path is the root directory.
	if (strcmp(path, "/") == 0) 
	{
		statbuf->st_mode = S_IFDIR | 0755;
		statbuf->st_nlink = 2;

		return 0; // no error
	}

	

	/*
	we need to create a few varibales which we will then copy to a majority of the methods!
 	size of file taking into account . /0
  	f extension--given in readme as example
   	f name-- given in readme as example 
    	^both used in others 
     	checked is used to check if the path is in the file 
	0,1,2
 	*/
	 //size 
	char directory[MAX_FILENAME+1];
	char fext[MAX_FILENAME+1];
	char fname[MAX_FILENAME+1];
	int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1
	int s;


	//time to check if path is in the file
	// Check if the path is a subdirectory.
	// 	if (path is a subdirectory) {
	// 		statbuf->st_mode = S_IFDIR | 0755;
	// 		statbuf->st_nlink = 2;
	//
	// 		return 0; // no error
	// 	}


	//ok now files 
	// Check if the path is a file.
	// 	if (path is a file) {
	// 		// Regular file
	// 		statbuf->st_mode = S_IFREG | 0666;
	//
	// 		// Only one hard link to this file
	// 		statbuf->st_nlink = 1;
	//
	// 		// File size -- replace this with the real size
	// 		statbuf->st_size = 0;
	//
	// 		return 0; // no error
	// 	}



	if(checked==2 || checked==3){//if path is a file 
		statbuf->st_mode=S_IFREG | 0666; //reg file type^given above
		statbuf->st_nlink=1;//1 link found ^given above 

		struct cs1550_directory_entry *curr_d= file_entried_into_this_p(directory);//this represents thr current directory we are on 

		if(curr_d)//if we find a match then lets find the file
		{
			/*
				we have to check if there is a match to directory then file 
    				specific error calls for this if not 

   			*/
			struct cs1550_file_entry *curr_f= where_dat_file_at(curr_d, fname, fext);
			if(curr_f)
				s=curr_f->fsize;//file size to size of file bc we have a match
			else {
				free(curr_d);//free the current directory 
				return -ENOENT;
			}
		}
		else //not found
			return -ENOENT;

		statbuf->st_size=s;//file size ^given above 
		free(curr_d);//free current directory 
		return 0;//return w no error found
		
	}

	if(checked==1)
	{//this checks if the path is in a subdirectory now 

		struct cs1550_directory_entry *curr_d=file_entried_into_this_p(directory);//current directory 

		if(curr_d){//this is all for if the path exists 

			statbuf->st_mode=S_IFDIR | 0755;//same as given above 
			statbuf->st_nlink=2;//same as given above 
			free(curr_d);//free space from current directory 
			return 0;//we always return 0 for correct 
		}
		else
			return -ENOENT;
			//return -ENOTDIR; 
	}

	// Otherwise, the path doesn't exist.
	return -ENOENT;
}

/**
 * Called whenever the contents of a directory are desired. Could be from `ls`,
 * or could even be when a user presses TAB to perform autocompletion.




 This function should list all subdirectories of the root, 
 or all files of a subdirectory (depending on the path).

0 on success
-ENOENT if the directory is not found


There is a slight problem w this where it crashes----


extension check, '/0' '.' 




 
 */
static int cs1550_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			  off_t offset, struct fuse_file_info *fi)
{

	(void)(offset);
	(void)(fi);

	if(get_pathed_b(path)==0)//false
		return -ENAMETOOLONG;//path is not valid so remove and exit 



	//int num= MAX_FILENAME+1; //size 
	char directory[MAX_FILENAME+1];
	char fext[MAX_FILENAME+1];
	char fname[MAX_FILENAME+1];
	//int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1
	//^copied from above

	
	int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1

	
	// This assumes no subdirectories exist. You'll need to change this.
	if (strcmp(path, "/") == 0)
	{//this checks the path to find the specific directory the specific list of files is in 
		
		filler(buf, ".", NULL, 0);//add the current directory 
		filler(buf, "..", NULL, 0);//add the parent directory 

		for(size_t t=0; t<root->num_directories; t++)
			{//start at root, list all the directories within
			filler(buf, root->directories[t].dname, NULL, 0);
			}
		return 0;//done 
	}
	else if(checked==1)
	{ //we are in a directory of a directory = subdirectory 
		
		filler(buf, ".", NULL, 0);//add current directory
		filler(buf, "..", NULL, 0);//add the parent directory 

		struct cs1550_directory_entry *curr_d=file_entried_into_this_p(directory);
		if(!curr_d)
		{
			return -ENOENT;
		}
		else 
		{//this means we found a match 
			/*
   			once we find a match we need to create a way to store the filename,
      			take into account the extension as well
	 		This means max of both extension and filename +1 for each
    			then, we fill it cpy filename and extension if exists 
       			free, done! 

   			*/
			//char array[MAX_FILENAME+1+MAX_EXTENSION+1];//+1 to both for . and /0
			char entry[MAX_FILENAME+MAX_EXTENSION+2];// ^same as above
			for(size_t t=0; t<curr_d->num_files; t++)
			{
				
				strncpy(entry, curr_d->files[t].fname, (MAX_FILENAME+1));

					if(strcmp(curr_d->files[t].fext, "")!=0)
					{
						strncat(entry, ".", 2);//this appends
					
						strncat(entry, curr_d->files[t].fext, (MAX_EXTENSION+1));
						//^this appends extension
				
			
					}
				
				filler(buf, entry+1, NULL, 0);//changes to buffer, we need to write to ac
			}
			free(curr_d);//free current directory 
			return 0;//return good
		}
		
	// The filler function allows us to add entries to the listing.
	
	 
	}
	else
		return -ENOENT;

	// Add the subdirectories or files.
	// The +1 hack skips the leading '/' on the filenames.
	//
	//	for (each filename or subdirectory in path) {
	//		filler(buf, filename + 1, NULL, 0);
	//	}

	//return 0;
}

/**
 * Creates a directory. Ignore `mode` since we're not dealing with permissions.


 This function should add the new directory to the root level, 
 and should update the.disk file appropriately.
 
 */
static int cs1550_mkdir(const char *path, mode_t mode)
{
	(void)(mode);

	//int num= MAX_FILENAME+1; //size 
	char directory[MAX_FILENAME+1];
	char fext[MAX_FILENAME+1];
	char fname[MAX_FILENAME+1];
	//int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1


	if(get_pathed_b(path)==0)//false
		return -ENAMETOOLONG;//path is not valid so remove and exit 

	int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1


	//^copied from above 

	if(checked==1)
	{//we are tryibg to make directory at root 
		for(size_t t=0; t < root->num_directories; t++)//loop through root directories
			if(strcmp(directory, root->directories[t].dname)==0)//if name matches
				return -EEXIST;//it exists 
		if(root->num_directories >= MAX_DIRS_IN_ROOT)//space exists 
			return -ENOSPC;
		else
		{//directory does not exist 
			/*
			   if the directory doesnt exist we have to create one and add so on. 
			   we ensure there is space before we add. cpy the directory anme 
			   use last allocated block and root to adjust 
			   new directory so we incremment by 1
			   start of file 
			   write changes 
			   success
      			*/
			strncpy(root->directories[root->num_directories].dname, directory, (MAX_FILENAME+1));
			//^cpy directory name to next num
			root->directories[root->num_directories].n_start_block=root->last_allocated_block+1;
			//^root is changed to last allocated 
			root->num_directories++;
			//^we added to increase by 1
			root->last_allocated_block++;
			//^""
			fseek(file, 0, SEEK_SET);
			fwrite(root, BLOCK_SIZE, 1, file);
			//^seek to start of the file and then we want to write bc we change 
			return 0;//return good
		}
	}

	return -EPERM;//nope not enough space/ cant 
	
}

/**
 * Removes a directory.
 */
static int cs1550_rmdir(const char *path)
{
	(void)(path);
	return 0;
}

/**
 * Does the actual creation of a file. `mode` and `dev` can be ignored.

 This function should add a new file to a subdirectory, 
 and should update the .disk file appropriately with the modified 
 directory entry structure.
 
 */
static int cs1550_mknod(const char *path, mode_t mode, dev_t dev)
{
	(void)(mode);
	(void)(dev);


	//int num= ; //size 
	char directory[MAX_FILENAME+1];
	char fext[MAX_FILENAME+1];
	char fname[MAX_FILENAME+1];
	//int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1

	if(get_pathed_b(path)==0){//false
		return -ENAMETOOLONG;//path is not valid so remove and exit 
	}

	int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1


	
	//^copied from above 

	if(checked==2 || checked==3)
	{//we need 2 or 3 arguments for this 
		struct cs1550_directory_entry *curr_d=file_entried_into_this_p(directory);
		if(!curr_d)
			return -ENOENT;//error 
		else
		{
			struct cs1550_file_entry *curr_f= where_dat_file_at(curr_d, fname, fext);

			if(!curr_f)//if the file does not exist 
			{
				/*
				    subdirectory time. same as method above kind of we check for space first 
				    if there is space then create the file subdirectory 
				    we need to first block of said directory abvoe 
				    extension adjustment 
				
				*/
				if(curr_d->num_files< MAX_FILES_IN_DIR)
				{//create and add file 
					int first=start_BLOCKed(directory);

					strncpy(curr_d->files[curr_d->num_files].fname,fname, (MAX_FILENAME+1));
					if(checked==3)
					{//add ext to file
						#pragma GCC diagnostic push
						#pragma GCC diagnostic ignored "-Wstringop-truncation"
						strcpy(curr_d->files[curr_d->num_files].fext,fext);
						#pragma GCC diagnostic pop
					}
					curr_d->files[curr_d->num_files].fsize=0;
					curr_d->files[curr_d->num_files].n_index_block=root->last_allocated_block+1;

					//write the val of the last allocated blockas the dirst entry. increment 
					/*
						Everything below is a mix of these things. 
						we seek to start. read the index varaible then use fwrite to 
						write the num to the last allocated
						block using the said method. This will become the first entry in the
						current block array which we are using for indexes! 
						we find the directory # by using the index block*block size 
						read the info and incremnet as needed
						DONT FORGET TO ALLOCATE!!!!
						
						then comes the disk 
						fseek and fwrite for current directory, first block
						, file, root, etc. 
						
						then we free the current block, directory, and file.
						success so 0
					*/
					struct cs1550_index_block *curr_b=malloc(sizeof(struct cs1550_index_block));
					memset(curr_b->entries, 0, sizeof(curr_b->entries));
					//memset(curr_b,0, sizeof(struct cs1550_index_block));
					fseek(file,((root->last_allocated_block+1)* BLOCK_SIZE), SEEK_SET);
					fread(curr_b, BLOCK_SIZE, 1, file);//fread data into block

					root->last_allocated_block++;//increment
					curr_b->entries[0]=root->last_allocated_block+1;//allocation
					root->last_allocated_block++;//increment 
					curr_d->num_files++;//increment 

					fseek(file, first*BLOCK_SIZE, SEEK_SET);//write back to disk
					fwrite(curr_d, BLOCK_SIZE, 1, file);

					fseek(file, 0, SEEK_SET);//write back to disk 
					fwrite(root, BLOCK_SIZE, 1, file);

					fseek(file, ((root->last_allocated_block-1)*BLOCK_SIZE), SEEK_SET);
					fwrite(curr_b, BLOCK_SIZE, 1, file);//write back to disk 

					free(curr_d);//free
					free(curr_f);
					free(curr_b);
					return 0;//return good 
				}
				else 
				{//not enough space 
					free(curr_d);
					free(curr_f);
					return -ENOSPC;
				}

			}
			else
			{//f 
				free(curr_d);//free the current directory, nothing else is used 
				free(curr_f);
				return -EEXIST;
			}
		}
	}
		return -EPERM;//boooooom 

}

/**
 * Deletes a file.
 */
static int cs1550_unlink(const char *path)
{
	(void)(path);
	//unlink or delete?
	return 0;
}

/**
 * Read `size` bytes from file into `buf`, starting from `offset`.
This function reads size bytes from the file into buf, starting at offset.

 */
static int cs1550_read(const char *path, char *buf, size_t size, off_t offset,
		       struct fuse_file_info *fi)
{
	(void)(fi);
	

	//int num= MAX_FILENAME+1; //size 
	char directory[MAX_FILENAME+1];
	char fext[MAX_FILENAME+1];
	char fname[MAX_FILENAME+1];
	//int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1

	if(get_pathed_b(path)==0)//false
		return -ENAMETOOLONG;//path is not valid so remove and exit 

	int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1

	if(checked==2 || checked==3){//we got a path and a filename 

		

		struct cs1550_directory_entry *curr_d=file_entried_into_this_p(directory);
		if(!curr_d)
			return -ENOENT;
		else {//we got a matching directory in da root block. 

			

			struct cs1550_file_entry *curr_f= where_dat_file_at(curr_d, fname, fext);
			if(curr_f)
			{//if we are in the current dir 

				
				struct cs1550_index_block *curr_b= malloc(sizeof(struct cs1550_index_block));
				fseek(file, (curr_f->n_index_block * BLOCK_SIZE), SEEK_SET);
				fread(curr_b, BLOCK_SIZE, 1, file);
				//read index block above, fseek and fread for block/index data
				//same as done above in other methods 

				//info is the data fom the current block 
				struct cs1550_data_block *info= malloc(sizeof(struct cs1550_data_block));

				size_t t=0;//set a temporary size which we will use and incemren 
				
				while(t!=size){
				/*
					the offset parameter allows us to find the index. 
					This is the index of the info blocks within the indexes. 
					then we need to find the current offset to we ac know which info to use
				*/

					int curr_i=(offset+t)/BLOCK_SIZE;//current_index
					int curr_o=(offset+t)% BLOCK_SIZE;//current_offset




					

					//this is where stuff gets tricky

					
					int curr_s=BLOCK_SIZE;//ok this is what we need to read in for the first iteration of the loop
					if((size-t)<BLOCK_SIZE)
					{//if we are less than ok good 
						if((size-t)+curr_o>BLOCK_SIZE)
							curr_s=BLOCK_SIZE-curr_o;//if we are greater than with the offset we calucalted above
							//then perf change size
						else
							curr_s=size-t;//else size intecment slowl 
					}

					if(curr_b->entries[curr_i]==0){//this is nothing it just checks to see if eindex is empty 
						t+=curr_s;
						continue;
					}
					//seek and read same as above!!!!

					fseek(file, (curr_b->entries[curr_i]*BLOCK_SIZE), SEEK_SET);
					fread(info, BLOCK_SIZE, 1, file);

					memcpy(buf+t, ((char*)info)+curr_o, curr_s);
					t+=curr_s;//t is the temporary size 


				}

				free(curr_d);//free current directory 
				free(curr_b);//free current block
				free(info);//free info from index 
				return size;//return size 

			}
			else
			{	
			free(curr_d);
			return -ENOENT;
			}

		}
		
	
}

	return -EISDIR;//error
	
	
}

/**
 * Write `size` bytes from `buf` into file, starting from `offset`.


 This function writes size bytes from buf into the file, 
 starting at offset.
 */
static int cs1550_write(const char *path, const char *buf, size_t size,
			off_t offset, struct fuse_file_info *fi)
{

	(void)(fi);

	

	//int num= MAX_FILENAME+1; //size 
	char directory[MAX_FILENAME+1];
	char fext[MAX_FILENAME+1];
	char fname[MAX_FILENAME+1];
	//int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1

	if(get_pathed_b(path)==0)//false
		return -ENAMETOOLONG;//path is not valid so remove and exit 

	int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1

	//struct cs1550_file_entry *curr_f=where_dat_file_at(curr_d, fname, fext);

	//^same structure as usual. check above its literally 
	//copy and paste 

	if(checked==2 || checked==3)//path and filename 
	{
		struct cs1550_directory_entry *curr_d=file_entried_into_this_p(directory);

		if(curr_d)//if we are in the current diirectory 
		{

			struct cs1550_file_entry *curr_f=where_dat_file_at(curr_d, fname, fext);
			if(!curr_f)//this is just checking for file 
			{
				free(curr_d);
				return -ENOENT;
			}
			else
			{//ok we have the file now 
				struct cs1550_index_block *curr_b= malloc(sizeof(struct cs1550_index_block));
				fseek(file, (curr_f->n_index_block*BLOCK_SIZE), SEEK_SET);
				fread(curr_b, BLOCK_SIZE, 1, file);
				//^this is same structure as above w current block for index 

				struct cs1550_data_block *info= malloc(sizeof(struct cs1550_data_block));
				//index info above 
				size_t t=0; //same temporary size variable 
				while(t!=size){

					int curr_i= (offset+t)/BLOCK_SIZE;//current index 
					int curr_o= (offset+t)%BLOCK_SIZE;//current offset 

					int curr_s=BLOCK_SIZE;//current size 
					if((size-t) < BLOCK_SIZE)
					{
						if((size-t)+curr_o > BLOCK_SIZE)
							curr_s=BLOCK_SIZE-curr_o;
						else
							curr_s=size-t;
					}

					//^this is basically the same as above 

					if(curr_b->entries[curr_i]==0)//oh no//index entry is empty 
					{
						curr_b->entries[curr_i]=root->last_allocated_block+1;
						root->last_allocated_block++;
						//this means there is space and we can allocate a new block 
						//for into 

						fseek(file, 0, SEEK_SET);
						fwrite(root, BLOCK_SIZE, 1, file);
						//find and write back to disk 

						fseek(file, (curr_f->n_index_block*BLOCK_SIZE), SEEK_SET);
						fwrite(curr_b, BLOCK_SIZE, 1, file);
						//find and write back to disk
					}

					fseek(file, (curr_b->entries[curr_i]*BLOCK_SIZE), SEEK_SET);
					fread(info, BLOCK_SIZE,1,file);
					//find and read info 

					
					memcpy(((char*) info)+curr_o, buf+t, curr_s);//copy info 

					fseek(file, (curr_b->entries[curr_i]* BLOCK_SIZE), SEEK_SET);
					fwrite(info, BLOCK_SIZE, 1, file);
					//find and write back to disk 

					t+=curr_s;//this is same as above increment temp size 
					
					
				}
				free(curr_b);//free the current block we r using 
				free(info);//free the info from the block


				//this is an error fix for tests 
				if(offset==0)//if the offset is 0 this means we are at the start of the file
					//overwrite fix for tests
					curr_f->fsize=size;
					//curr_f->fsize=curr_f->fsize+size;
				else
					curr_f->fsize+=size;//not at the beginning

				fseek(file, (start_BLOCKed(directory)*BLOCK_SIZE), SEEK_SET);
				fwrite(curr_d, BLOCK_SIZE, 1, file);
				//find and write back to disk 
				
				free(curr_d);//free current directory 
				return size;
			}
		}
		else
			return -ENOENT;
		
	}


	return -EISDIR;
	
	
}

/**
 * Called when a new file is created (with a 0 size) or when an existing file
 * is made shorter. We're not handling deleting files or truncating existing
 * ones, so all we need to do here is to initialize the appropriate directory
 * entry.
 */
static int cs1550_truncate(const char *path, off_t size)
{
	(void)(path);
	(void)(size);
	return 0;
}

/**
 * Called when we open a file.
 This function should verify that the input path exists.
 */
static int cs1550_open(const char *path, struct fuse_file_info *fi)
{
        // If we can't find the desired file, return an error
        (void)(fi);

	//int num= MAX_FILENAME+1; //size 
	char directory[MAX_FILENAME+1];
	char fext[MAX_FILENAME+1];
	char fname[MAX_FILENAME+1];

	if(get_pathed_b(path)==0)//false
		return -ENAMETOOLONG;//path is not valid so remove and exit 


	int checked=sscanf(path, "/%[^/]%[^.].%s", directory, fname, fext);//2,3,1

	//^copied from above 

	if(checked==1){//subdirectory
		struct cs1550_directory_entry *curr_d=file_entried_into_this_p(directory);
		if(!curr_d)//we are in curertn direcotry is exists 
			return -ENOENT;//return good 
		else
			return 0;//ekse no return error 

	}
	else if(checked==2 || checked==3){//this is for filename and path 
		struct cs1550_directory_entry *curr_d=file_entried_into_this_p(directory);

		if(curr_d)
		{//if we r in current directory 
			struct cs1550_file_entry *curr_f=where_dat_file_at(curr_d, fname, fext);
			if(curr_f)//if we r in file 
				return 0;//return good 
			else
				return -ENOENT; //return error 
		}
		else
			return -ENOENT;
	}
	else
		return -ENOENT;//return error 

	
}

/**
 * Called when close is called on a file descriptor, but because it might
 * have been dup'ed, this isn't a guarantee we won't ever need the file
 * again. For us, return success simply to avoid the unimplemented error
 * in the debug log.
 */
static int cs1550_flush(const char *path, struct fuse_file_info *fi)
{
	// Success!
	(void)(path);
	(void)(fi);
	return 0;
}

/**
 * This function should be used to open and/or initialize your `.disk` file.
 This function includes code (e.g., opening the .disk file) 
 that is run when the file system loads.
 */
static void *cs1550_init(struct fuse_conn_info *fi)
{
	// Add your initialization routine here.

	(void)(fi);
	root=malloc(BLOCK_SIZE);

	file=fopen(".disk", "rb+");
	if(file!=NULL)
		fread(root, BLOCK_SIZE, 1, file);



	return NULL;
}

/**
 * This function should be used to close the `.disk` file.
 */
static void cs1550_destroy(void *args)
{
	// Add your teardown routine here.

	(void)(args);
	free(root);//free da root 
	fclose(file);//close the file
}

/*
 * Register our new functions as the implementations of the syscalls.
 */
static struct fuse_operations cs1550_oper = {
	.getattr	= cs1550_getattr,
	.readdir	= cs1550_readdir,
	.mkdir		= cs1550_mkdir,
	.rmdir		= cs1550_rmdir,
	.read		= cs1550_read,
	.write		= cs1550_write,
	.mknod		= cs1550_mknod,
	.unlink		= cs1550_unlink,
	.truncate	= cs1550_truncate,
	.flush		= cs1550_flush,
	.open		= cs1550_open,
	.init		= cs1550_init,
	.destroy	= cs1550_destroy,
};

/*
 * Don't change this.
 */
int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &cs1550_oper, NULL);
}


////////////////////////////////HELPER METHODS//////////////////////////////////
static struct cs1550_directory_entry *file_entried_into_this_p(char name[]){

	for(size_t t=0; t<root->num_directories; t++){ //loop through the root directories
		//stop at find 

		if(strcmp(name, root->directories[t].dname)==0){ //match
			int block_num= root->directories[t].n_start_block;//copied block # 
			
			struct cs1550_directory_entry *directory = malloc(sizeof(struct cs1550_directory_entry));
			//^this is a pointer to the corresponding entry for the directory

			//fseek---->CHECK UP TOP  
			fseek(file, block_num * BLOCK_SIZE, SEEK_SET);
			//fread--->CHECK UP TOP
			fread(directory, BLOCK_SIZE, 1, file);

			return directory;//returns said directory for path
		}
	}

	return NULL;//return null bc no directory found 
}

static int start_BLOCKed(char name[]){//this is all for the first block aka the start block

	for(size_t t=0; t<root->num_directories; t++) //loop through same as above 
		if(strcmp(name, root->directories[t].dname)==0)//match, same as above
			return root->directories[t].n_start_block; //same as above! 
			//^copies block #

	return 0; //no match is found for start block
}


static struct cs1550_file_entry *where_dat_file_at(struct cs1550_directory_entry *directory, char fname[], char ext[]){


	for(size_t t=0; t<directory->num_files; t++)// loop trhough all files
		if((strcmp(fname, directory->files[t].fname)==0)&& (strcmp(ext, directory->files[t].fext)==0))//match
			return &(directory->files[t]);//return file if matched correctly 


	return NULL; //no match is found for file 
}

static int get_pathed_b(const char *p){

	//this makes sure the path is valid by checking the arguents given so we dont have to do it in method 

	int valid=0;//get it valid ends in d for directory 
	int valif=0;//this one ends in f for file ie valid file 
	int beepboop=0;//i got lazy 

	for(int i=1; i<(MAX_FILENAME+2); i++){//dis is the directory part of file,path 
		if(p[i]=='/'){ //ayo this is valid
			valid=1;
			beepboop=i; //this is the index 
			break;
		}
		else if(p[i]=='\0')//also NULL
			return 1; 
	}

	if(valid==0)//check if the directory is valid bc if not return 0
		return 0;

	for(int i=beepboop+1; i<(beepboop+MAX_FILENAME+2);i++){

		if(p[i]=='.'){//this means filename is valid 
			valif=1;
			beepboop=i; //index 
			break;
		}
		else if(p[i]=='\0')//same as above 
			return 1;
	}

	if(valif==0)//make sure file name is valid
		return 0;

	for(int i=beepboop+1; i<(beepboop+MAX_EXTENSION+2);i++){ //now this is cyc;ing through the file until end of path

		if(p[i]=='\0')//null then valid? kinda weird but ext is valid so path is 
			return 1;

	}

	return 0; //else nope it is not 

	
}
