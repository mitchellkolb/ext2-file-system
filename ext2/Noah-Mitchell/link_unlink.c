//link_unlink.c file

extern MINODE *root;
extern PROC proc[NPROC], *running;
extern int fd, dev, requests;


void link(char *pathname, char *parameter) {
    MINODE *mip, *lpmip;
    
    mip = path2inode(pathname);
    if (!mip) {
    	printf("Error: pathname invalid\n");
    	return -1;
    }

    if (!S_ISREG(mip->INODE.i_mode)) {
        printf("ERROR: PATH IS NOT A REGULAR FILE\n");
        iput(mip);
        return;
    }

    // Get parent and child pathnames
    char linkParentPath[256], linkChildPath[256];
    strcpy(linkParentPath, dirname(parameter));
    strcpy(linkChildPath, basename(parameter));

    // Check if link already exists
    lpmip = path2inode(linkParentPath);

    if (search(lpmip, linkChildPath) != 0) {
        printf("ERROR: LINK ALREADY EXISTS\n");
        iput(lpmip);
        iput(mip);
        return;
    }

    // Create new link and update inode
    enter_child(lpmip, mip->ino, linkChildPath);
    mip->INODE.i_links_count++;
    mip->modified = 1;
    lpmip->modified = 1;

    iput(mip);
    iput(lpmip);
}


int unlink(char* pathname)
{
    MINODE *mip;

    char parent[256], child[256];
    strcpy(parent, dirname(pathname));
    strcpy(child, basename(pathname));

    mip = path2inode(pathname);
    if (!mip) {
    	printf("Error obtaining path\n");
    	return -1;
    }

    // Ensure file is not a directory
    if (S_ISDIR(mip->INODE.i_mode)) {
        printf("dir cannot be link; cannot unlink %s\n", pathname);
        return -1;
    }

    mip->INODE.i_links_count--;
    if (mip->INODE.i_links_count == 0) {
        // deallocate data blocks with truncate() function
        if (!S_ISLNK(mip->INODE.i_mode)) {
            truncate(mip);
        }
    }
    mip->modified = 1;
    iput(mip);

    // now remove child
    // parent MIP
    MINODE *pip = path2inode(parent);

    char temp[256];
    strcpy(temp, child);

    rm_child(pip, temp);
    pip->modified = 1;
    iput(pip);
}

