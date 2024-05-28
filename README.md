


<h1 align="center">Ext2 File System</h1>

<p align="center">
  <img alt="Github top language" src="https://img.shields.io/github/languages/top/mitchellkolb/ext2-file-system?color=558c5d">

  <img alt="Github language count" src="https://img.shields.io/github/languages/count/mitchellkolb/ext2-file-system?color=558c5d">

  <img alt="Repository size" src="https://img.shields.io/github/repo-size/mitchellkolb/ext2-file-system?color=558c5d">

  <img alt="Github stars" src="https://img.shields.io/github/stars/mitchellkolb/ext2-file-system?color=558c5d" />
</p>

<p align="center">
    <img
        src="https://img.shields.io/badge/Programming Language-%2300599b?style=for-the-badge&logo=C&logoColor=white"
        alt="Website Badge" />
    <img
        src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=Linux&logoColor=black"
        alt="Website Badge" />
    <img
        src="https://img.shields.io/badge/Ext2 File System-558c5d?style=for-the-badge&logo=Educative&logoColor=white"
        alt="Website Badge" />

</p>

This project is intended to be a  Linux-compatible ext2 file system. The project deliverables were comprised of two primary sections. The first level was expanding the base file system to implement the file system tree. Second level focuses on implementing read and write operations of file contents.

![ext2-examplt-gif](resources/360-gif-2.gif)

<details>
<summary style="color:#5087dd">Watch the Full Video Demo Here</summary>

[![Full Video Demo Here](https://img.youtube.com/vi/L8SBFNDYORU/0.jpg)](https://www.youtube.com/watch?v=L8SBFNDYORU)

</details>


---

## Table of Contents
- [What I Learned](#what-i-learned-in-this-project)
- [Tools Used in this Project](#tools-used-in-this-project)
- [Development Environment](#development-environment)
- [Team / Contributors / Teachers](#team--contributors--teachers)
- [How to Set Up](#how-to-set-up)
- [Commands Implemented](#commands-implemented)
    - [Level 1](#level-1)
    - [Level 2](#level-2)
- [Acknowledgments](#acknowledgments)

---

# What I Learned in this Project
- Extreme detail of the Linux Ext2 File System
- Techniques for programming in C using data structures and file operations
- How to create and use shell scripts and virtual disks
- What all these topics are: Inodes, MBR (Master Boot Record), Super Block, Group/File Descriptor, Bitmap, Indirect Blocks, Forks/Pipes, and Mailman's Algorithm.

# Tools Used in this Project

- C programming language
- Systems Programming in Unix/Linux Textbook
- Linux distribution (Ubuntu) used in a Virtual Machine
- Ext2 file system library
- WSU Managed Github Repository

# Development Environment

- VS Code
- Terminal
- Ubuntu Version 20.04.3-desktop

# Team / Contributors / Teachers

- [Mitchell Kolb](https://github.com/mitchellkolb)
- [Noah Waxman](https://github.com/noah-waxman)
- Professor. K.C. Wang
- [Logan Kloft](https://github.com/LoganKloft) (Teachers Assistant)

# How to Set Up

This project was implemented on our local machine inside of a virtual machine using:

- Ubuntu Version 20.04.03 found at this [link](http://lt.releases.ubuntu.com/20.04.3/).
- Clone this repository 
- Open terminal at the codebase `~.../ext2/Noah-Mitchell/`
- Run our `mk` file to create a new disk image, and then start up the file system with that new disk image mounted.
- The Ext2 File System will begin running in your terminal. 


# Commands Implemented


| Symbol | Meaning |
|----|------------------------|
| ✅ | Implemented |
| ☑️ | Partially Implemented  |
| ❌ | Not Implemented  |

## Level 1

Implements the file system. The structure is described on page 318 of the [Systems Programming Book](https://link.springer.com/book/10.1007/978-3-319-92429-8). Also as a reference, the figure is below:

![Tree Structure](./resources/imgs/readme-figure.JPG)


<table>
    <thead>
        <tr>
            <th>Command</th>
            <th>Implemented?</th>
            <th>Description</th>
        </tr>
    </thead>
    <tbody>
         <tr>
            <th>cd</th>
            <th>✅</th>
            <th>
                Changes the directory from the current working directory to the
                new path.
            </th>
        </tr>
        <tr>
            <th>ls</th>
            <th>✅</th>
            <th>
                Lists information about the FILEs (only works for the current directory).
            </th>
        </tr>
        <tr>
            <th>pwd</th>
            <th>✅</th>
            <th>
                Prints the full filename of the current working directory.
            </th>
        </tr>
        <tr>
            <th>mkdir</th>
            <th>✅</th>
            <th>
                Creates a DIRECTORY, if they do not already exist.
            </th>
        </tr>
        <tr>
            <th>rmdir</th>
            <th>✅</th>
            <th>
                Removes the specified DIRECTORY, if its empty.
            </th>
        </tr>
        <tr>
            <th>creat</th>
            <th>✅</th>
            <th>
                Creates a FILE, if it does not already exist.
            </th>
        </tr>
        <tr>
            <th>show</th>
            <th>✅</th>
            <th>
                Prints the inode number, record length, name length, and name of each file item in the current working directory.
            </th>
        </tr>
        <tr>
            <th>hits</th>
            <th>✅</th>
            <th>
                Prints the cacheList and hit_ratio percentage.
            </th>
        </tr>
        <tr>
            <th>link</th>
            <th>☑️</th>
            <th>
                Creates a hard link to an existing file. (Bug: only works on files added when the disk image is created and not for any files created while running)
            </th>
        </tr>
        <tr>
            <th>unlink</th>
            <th>☑️</th>
            <th>
                Removes the hard link by decreasing the link count of the specified file. (Bug: only works on files added when the disk image is created and not for any files created while running)
            </th>
        </tr>
        <tr>
            <th>symlink</th>
            <th>☑️</th>
            <th>
                Creates a symbolic link that can link to DIRs and files not on the same device. (Bug: only works on files added when the disk image is created and not for any files created while running)
            </th>
        </tr>
        <tr>
            <th>exit</th>
            <th>✅</th>
            <th>
                Closes the file system process and all active file descriptors if any.
            </th>
        </tr>
    </tbody>
</table>


## Level 2

Focuses on read and write operations of files

<table>
    <thead>
        <tr>
            <th>Command</th>
            <th>Implemented?</th>
            <th>Description</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <th>open</th>
            <th>✅</th>
            <th>
                Opens a file descriptor for the specified file.
            </th>
        </tr>
        <tr>
            <th>pfd</th>
            <th>✅</th>
            <th>
                Prints the terminal all the opened file descriptors.
            </th>
        </tr>
        <tr>
            <th>close</th>
            <th>✅</th>
            <th>
                Closes a file descriptor for the specified file.
            </th>
        </tr>
        <tr>
            <th>lseek</th>
            <th>✅</th>
            <th>
                Repositions the offset of the file descriptor to the new position.
            </th>
        </tr>
        <tr>
            <th>read</th>
            <th>✅</th>
            <th>
                Reads nbytes from an opened file descriptor into a buffer area in user space.
            </th>
        </tr>
        <tr>
            <th>write</th>
            <th>☑️</th>
            <th>
                Writes nbytes from a buffer area in user space to an opened file descriptor and returns the actual number of bytes written. (Bug: Output is not saved once command completes)
            </th>
        </tr>
        <tr>
            <th>cat</th>
            <th>☑️</th>
            <th>
                Prints out the contents of the specified file until EOF is reached. (Bug: Segmenation fault for files with contents larger than 1024 bytes)
            </th>
        </tr>
        <tr>
            <th>head</th>
            <th>☑️</th>
            <th>
                Prints out the FIRST 10 lines of a file to the terminal. (Bug: Inconsistent output depending on what file is selected)
            </th>
        </tr>
            <th>tail</th>
            <th>☑️</th>
            <th>
                Prints out the LAST 10 lines of a file to the terminal. (Bug: Inconsistent output depending on what file is selected)
            </th>
        </tr>
            <th>cp</th>
            <th>❌</th>
            <th>
                Writes the contents of an existing file to a new area on the disk image, creating a duplicate of the file.
            </th>
        </tr>
            <th>mv</th>
            <th>❌</th>
            <th>
                Moves the file to a new space on current disk image or a new device entirely by creating a hard link or using the cp command if it's not the same dev.
            </th>
        </tr>        
    </tbody>
</table>


--- 
# Acknowledgments
This codebase and all supporting materials was made as apart of a course for my undergrad at WSU for CPTS 360 - Systems Programming C/C++ in the Spring of 2023. This project was originally submitted to a private repository as all WSU assignments are, that has forking disabled. This repository serves as a backup place to showcase this project. The original repo is [linked here.](https://github.com/CPTS-360-SPRING-2023/lab5-Mitchell-kolb/tree/main)