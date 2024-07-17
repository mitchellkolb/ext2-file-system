/*********** globals in main.c ***********/
extern PROC   proc[NPROC];
extern PROC   *running;

extern MINODE minode[NMINODE];   // minodes
extern MINODE *freeList;         // free minodes list
extern MINODE *cacheList;        // cacheCount minodes list

extern MINODE *root;

extern OFT    oft[NOFT];

extern char gline[256];   // global line hold token strings of pathname
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings                    

extern int ninodes, nblocks;
extern int bmap, imap, inodes_start, iblk;  // bitmap, inodes block numbers

extern int  fd, dev;
extern char cmd[16], pathname[128], parameter[128];
extern int  requests, hits, inodes_start, misses;

int open_file2(char* pathname, int mode) {
	// Mode = 0|1|2|3 for R|W|RW|APPEND

    //int value = atoi(mode);
	MINODE* mip = path2inode(pathname);

	if (!mip) {
        printf("File does not exist at pathname: %s\n", pathname);

        // READ: File must exist
        if (mode == 0) {
        	return -1;
        }

        creat_file(pathname);
        mip = path2inode(pathname);
        printf("mip dev=%d ino=%d\n", mip->dev, mip->ino);
    }

    

    // Check if the file is a regular file
    if (!S_ISREG(mip->INODE.i_mode)) {
    	printf("%s is not a regular file\n", pathname);
    	return -1;
    }

    // Search the oft array to see if the file is already opened
    int i;
    for (int i = 0; i < NOFT; i++) {
	    // Check if the OFT entry is in use
	    if (oft[i].shareCount > 0 && 
	    	oft[i].inodeptr->dev == mip->dev && 
	    	oft[i].inodeptr->ino == mip->ino) {
	        // If the file is opened for writing, RW or APPEND modes, reject the open request
	        if (oft[i].mode == 1 || oft[i].mode == 2 || oft[i].mode == 3) {
	            printf("File is already opened with an incompatible mode.\n");
	            return -1;
	        }
	    }
	}

    // File is not opened yet, find a free slot in the oft array
    for (i = 0; i < NOFT; i++) {
        if (oft[i].shareCount == 0) {
            break;
        }
    }

    
    // Initialize a new oft entry
    OFT* oftp = &oft[i];
    oftp->mode = mode;
    oftp->shareCount = 1;
    oftp->minodeptr = mip;

    // Adjust the file offset based on the mode
    switch (mode) {
        case 0: // READ
            oftp->offset = 0;
            break;
        case 1: // WRITE
        	truncate(mip);
        	oftp->offset = 0;
        case 2: // READ/WRITE
            oftp->offset = 0;
            break;
        case 3: // APPEND
            oftp->offset = mip->INODE.i_size;
            break;
        default:
            printf("invalid mode\n");
            return -1;
    }

    // Find the SMALLEST index i in running PROC's fd[ ] such that fd[i] is NULL
    for (i = 0; i < NFD; i++) {
        if (!running->fd[i]) {
            break;
        }
    }

    // Let running->fd[i] point at the OFT entry
    if (i >= NFD) {
        printf("no more free file descriptors\n");
        return -1;
    }
    running->fd[i] = oftp;

    // Update the time fields of the INODE based on the mode
    switch (mode) {
        case 0: // READ
        case 1: // WRITE
        case 2: // READ/WRITE
            mip->INODE.i_atime = time(0L); // touch atime
            mip->modified = 1; // mark the INODE as modified
            break;
        case 3: // APPEND
            mip->INODE.i_atime = mip->INODE.i_mtime = time(0L); // touch atime and mtime
            mip->modified = 1; // mark the INODE as modified
            break;
        default:
            printf("invalid mode\n");
            return -1;
    }

    return i;
}

int open_file(char* pathname, char* mode) {
    // Mode = 0|1|2|3 for R|W|RW|APPEND
    char flag[5];



    int value = atoi(mode);
    MINODE* mip = path2inode(pathname);

    if (!mip) {
        printf("File does not exist at pathname: %s\n", pathname);

        // READ: File must exist
        if (mode == 0) {
            return -1;
        }

        creat_file(pathname);
        mip = path2inode(pathname);
        printf("mip dev=%d ino=%d\n", mip->dev, mip->ino);
    }

    

    // Check if the file is a regular file
    if (!S_ISREG(mip->INODE.i_mode)) {
        printf("%s is not a regular file\n", pathname);
        return -1;
    }

    // Search the oft array to see if the file is already opened
    int i;
    for (int i = 0; i < NOFT; i++) {
        // Check if the OFT entry is in use
        if (oft[i].shareCount > 0 && 
            oft[i].inodeptr->dev == mip->dev && 
            oft[i].inodeptr->ino == mip->ino) {
            // If the file is opened for writing, RW or APPEND modes, reject the open request
            if (oft[i].mode == 1 || oft[i].mode == 2 || oft[i].mode == 3) {
                printf("File is already opened with an incompatible mode.\n");
                return -1;
            }
        }
    }

    // File is not opened yet, find a free slot in the oft array
    for (i = 0; i < NOFT; i++) {
        if (oft[i].shareCount == 0) {
            break;
        }
    }

    
    // Initialize a new oft entry
    OFT* oftp = &oft[i];
    oftp->mode = value;
    oftp->shareCount = 1;
    oftp->minodeptr = mip;

    // Adjust the file offset based on the mode
    switch (value) {
        case 0: // READ
            oftp->offset = 0;
            break;
        case 1: // WRITE
            truncate(mip);
            oftp->offset = 0;
        case 2: // READ/WRITE
            oftp->offset = 0;
            break;
        case 3: // APPEND
            oftp->offset = mip->INODE.i_size;
            break;
        default:
            printf("invalid mode\n");
            return -1;
    }

    // Find the SMALLEST index i in running PROC's fd[ ] such that fd[i] is NULL
    for (i = 0; i < NFD; i++) {
        if (!running->fd[i]) {
            break;
        }
    }

    // Let running->fd[i] point at the OFT entry
    if (i >= NFD) {
        printf("no more free file descriptors\n");
        return -1;
    }
    running->fd[i] = oftp;

    // Update the time fields of the INODE based on the mode
    switch (value) {
        case 0: // READ
        case 1: // WRITE
        case 2: // READ/WRITE
            mip->INODE.i_atime = time(0L); // touch atime
            mip->modified = 1; // mark the INODE as modified
            break;
        case 3: // APPEND
            mip->INODE.i_atime = mip->INODE.i_mtime = time(0L); // touch atime and mtime
            mip->modified = 1; // mark the INODE as modified
            break;
        default:
            printf("invalid mode\n");
            return -1;
    }

    return i;
}




int close_file(char* _fd) {
	// Convert fd to int
	int fd = atoi(_fd);

	// Verify fd is within range
	if (fd < 0 || fd >= NFD) {
		printf("File descriptor not within range\n");
		return -1;
	}

	OFT *oftp = running->fd[fd];
    running->fd[fd] = 0;

    oftp->shareCount--;
    if (oftp->shareCount > 0)
       return 0;

    // last user of this OFT entry ==> dispose of its minode
    MINODE *mip = oftp->minodeptr;
    mip->modified = 1;
    iput(mip);

    return 0; 
}

int my_lseek(char* _fd, char* _position) {
	int fd = atoi(_fd);
	int position = atoi(_position);
    // Verify fd is within range
    if (fd < 0 || fd >= NFD) {
        printf("File descriptor not within range\n");
        return -1;
    }

    // Check if entry exists
    if (running->fd[fd] == NULL) {
        printf("lseek error: No OFT entry\n");
        return -1;
    }

    // Get the MINODE pointer associated with the given fd
    MINODE *mip = running->fd[fd]->minodeptr;

    // Determine the current position within the file
    int current_position = running->fd[fd]->offset;

    // Determine the size of the file
    int file_size = mip->INODE.i_size;

    // Calculate the new position within the file
    int new_position = position;
    if (new_position < 0) {
        new_position = 0;
    } else if (new_position > file_size) {
        new_position = file_size;
    }

    // Set the OFT entry's offset to the new position
    running->fd[fd]->offset = new_position;

    // Return the new position
    return new_position;
}

int pfd()
{
  printf("fd     mode    offset   INODE\n");
  printf("---    ----    ------   -----\n");

  // Loop through all the entries in the oft array
  for (int i = 0; i < NOFT; i++) {
    OFT *oftp = &oft[i];

    // Skip empty entries
    if (!oftp->shareCount) {
    	continue;
    }

    // Print the file descriptor, mode, and offset
    printf("%-5d  ", i);
    switch (oftp->mode) {
      case 0:
        printf("READ    ");
        break;
      case 1:
        printf("WRITE   ");
        break;
      case 2:
        printf("RDWR    ");
        break;
      case 3:
        printf("APPEND  ");
        break;
      default:
        printf("UNKNOWN  ");
        break;
    }
    printf("%-8d", oftp->offset);

    // Print the device number and inode number of the corresponding MINODE
    MINODE *mip = oftp->minodeptr;
    printf("[%d,%d]\n", mip->dev, mip->ino);
  }

  return 0;
}

int dup(char* _fd) {
	int fd = atoi(_fd);
	// Verify fd is within range
    if (fd < 0 || fd >= NFD) {
        printf("File descriptor not within range\n");
        return -1;
    }

    // Check if fd is already opened
    if (running->fd[fd] != NULL)
    {
        // Increment the share count of the OFT
        running->fd[fd]->shareCount++;
        
        // Find the first available file descriptor and return it
        for (int i = 0; i < NFD; i++)
        {
            if (running->fd[i] == NULL)
            {
                running->fd[i] = running->fd[fd];
                return i;
            }
        }
        
        // If all file descriptors are used, return an error
        printf("Error: No available file descriptors\n");
        return -1;
    }
    else
    {
        printf("Error: File descriptor %d is not opened\n", fd);
        return -1;
    }
}

int dup2(char* _fd, char* _gd) {
	int fd = atoi(_fd);
	int gd = atoi(_gd);

	// Verify fd is within range
    if (fd < 0 || fd >= NFD) {
        printf("File descriptor not within range\n");
        return -1;
    }

    // Verify gd is within range
    if (gd < 0 || gd >= NFD) {
        printf("File descriptor not within range\n");
        return -1;
    }

	// Check if gd is opened
    if (running->fd[gd] != NULL) {
    	// Close file if gd is opened
    	if (close_file(gd) != -1) {
    		printf("dup2: Error closing file\n");
    		return -1;
    	}
    }

    // dupe fd[fd] into fd[gd]
    OFT *oftp = running->fd[fd];
    running->fd[gd] = oftp;

    // Increase shareCount
    oftp->shareCount++;

    return 0;
}
