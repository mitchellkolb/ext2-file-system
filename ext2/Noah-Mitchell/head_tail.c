



void head(char *filename)
{
    //printf("This is the filename for head: %s\n", filename);


//How to head file: Linux head prints the FIRST 10 lines of a file
//1. open the file for READ;
    int fd = open_file2(filename, 0);
    char buf[BLKSIZE];
    int n, lines = 0;

//2. read BLKSIZE into char buf[ ] = |abcd..\n1234...\nxyzw...\n....
    while ((n = myread(fd, buf, BLKSIZE))) 
    {
        char *cp = buf;
        char *last_newline = NULL;

//3. scan buf[ ] for \n; For each \n: lines++ until 10; add a 0 after LAST \n
        for (int i = 0; i < n; i++) 
        {
            if (buf[i] == '\n') 
            {
                lines++; //Counts 10 lines after the 10th line it adds the NULL char to it thus ending the string 
                if (lines > 10) 
                {
                    if (last_newline != NULL) 
                    {
                        *last_newline = '\0';
                        printf("%s", buf);
                    }
                    close_file(fd);
                    return;
                }
                last_newline = &buf[i];
            }
        }
//4. print buf as %s
        printf("%.*s", n, buf);
    }

    close_file(fd);
}



void tail(char *filename)
{
	//printf("This is the filename for tail: %s\n", filename);

// HOW to tail file: Linux tail prints LAST 10 lines of a file
						
    char buf[BLKSIZE], *cp;
    int n, fd, file_size, lines_read = 0;

// 1. open file fore READ;
    fd = open_file2(filename, 0);
    if (fd < 0) {
        printf("Failed to open file '%s'\n", filename);
        return;
    }

// 2. get file_size (in its INODE.i_size)
    // Get the MINODE pointer associated with the given fd
    MINODE *mip = running->fd[fd]->minodeptr;

    // Determine the size of the file
    file_size = mip->INODE.i_size;

    // Seek to the starting point in the file
    int offset = (file_size > BLKSIZE) ? file_size - BLKSIZE : 0;
// 3. lssek to (file_size - BLKSIZE)      (OR to 0 if file_size < BLKSIZE)
    my_lseek(fd, offset);

// 4. n = read BLKSIZE into buf[ ]=|............abc\n1234\nlast\n|0|
    n = myread(fd, buf, BLKSIZE);
    buf[n] = '\0';

// 5. scan buf[ ] backwards for \n;  lines++  until lines=11
    // Scan buffer backwards for newline characters
    for (cp = buf + strlen(buf) - 1; cp >= buf; cp--) {
        if (*cp == '\n') {
            lines_read++;
            if (lines_read == 11) {
                // Print the remaining part of the buffer
                printf("%s", cp + 1);
                break;
            }
        }
    }

// 6. print from cp+1 as %s
    if (lines_read < 11) {
        printf("%s", buf);
    }

    close_file(fd);
}


