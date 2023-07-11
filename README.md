# xv6 Virtual File System (ext2) Implementation

This GitHub repository is an implementation of a virtual file system, the ext2 file system in the xv6 operating system, forked from https://github.com/mit-pdos/xv6-public. 

The goal is to provide a basic understanding of file system concepts and demonstrate how to integrate a file system into an existing operating system.

## Virtual File System (VFS)

The Virtual File System (VFS) is an abstraction layer that provides a unified interface for file system operations in an operating system. It allows applications and system components to interact with various file systems without being aware of the underlying implementation details.

The key purpose of VFS is to decouple the file system implementation from the rest of the operating system. By defining a common set of operations and data structures, VFS enables the operating system to support multiple file systems simultaneously. This flexibility allows users to choose different file systems based on their specific needs, such as performance, reliability, or special features.

VFS provides a standardized interface for operations like file creation, deletion, opening, reading, and writing. It also handles directory operations, such as creating, removing, and listing directories. By presenting a uniform API to applications, VFS simplifies the development of file-related functionality and enhances portability across different file systems.

## Ext2 File System

The Extended File System 2 (ext2) is a widely used file system in Linux systems. It was introduced as an improvement over the original ext file system, providing enhanced performance, reliability, and support for larger file systems.

Some key features of the ext2 file system include:

1. **Inode-based Structure**: The ext2 file system organizes files and directories using inodes. Each file or directory is represented by an inode, which stores metadata like permissions, timestamps, and pointers to data blocks.

2. **Block Grouping**: The file system divides the disk into block groups, with each group containing a set of inodes and data blocks. This grouping improves performance by reducing disk seek times.

3. **Hierarchical Directory Structure**: The ext2 file system supports a hierarchical directory structure, allowing files and directories to be organized in a tree-like format.

4. **File and Directory Permissions**: Permissions in ext2 determine the access rights for users and groups, enabling fine-grained control over file system security.

5. **Journaling**: While the original ext2 file system did not include journaling, later versions (ext3 and ext4) introduced journaling capabilities to enhance data consistency in the event of system crashes or power failures.

The ext2 file system provides a reliable and efficient solution for storing and managing files in Linux-based operating systems. Its design principles and features have influenced subsequent file systems, making it an essential component of the Linux ecosystem.

## Integration of ext2 with xv6

The xv6 operating system, based on the Sixth Edition of Unix, is a simple educational operating system. Integrating the ext2 file system into xv6 involves implementing the ext2 file system operations, modifying the file system interface, system calls, and user commands.

The integration process requires mapping the concepts and structures of the ext2 file system to the appropriate components in xv6. This includes managing disk blocks, inodes, directory entries, file permissions, and implementing operations like file creation, deletion, opening, reading, and writing.

Implementing ext2 in xv6 enables users to experiment with file system operations, explore the interactions between the file system and the rest of the operating system, and gain a deeper understanding of how file systems impact overall system performance and functionality.
## Getting Started

To get started with the xv6-ext2 project, follow these steps:

1. **Clone the Repository**: Clone this repository to your local machine.
   ```
   git clone https://github.com/TanmayPatil105/xv6-ext2
   ```

2. **Build xv6**: Create a temporary directory to mount the ext2.img on it
   ```
   mkdir /tmp/1  
   ```
   Now build the xv6 os by using the following command
   ```
   make clean qemu
   ```
   If it asks for a sudo password while building, provide it with your sudo password! :)
   
4. **Explore xv6**: Run the modified xv6 operating system with the above command. Test the file system operations and verify their correctness.
   by running the **ext2fstest**
   
5. **Contribute**: Explore the project code, understand the implementation details, and contribute to the project by fixing bugs or adding new features.







