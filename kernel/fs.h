// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
	uint size;         // Size of file system image (blocks)
	uint nblocks;      // Number of data blocks
	uint ninodes;      // Number of inodes.
	uint nlog;         // Number of log blocks
	uint logstart;     // Block number of first log block
	uint inodestart;   // Block number of first inode block
	uint bmapstart;    // Block number of first free map block
};

#define NDIRECT 11
#define NINDIRECT (BSIZE / sizeof(uint))
#define NDOUBLEINDIRECT NINDIRECT*NINDIRECT
#define BLOCK_SIZE 128
#define MAXFILE (NDIRECT + NINDIRECT + NDOUBLEINDIRECT)

// On-disk inode structure
struct dinode {
	short type;           // File type da li je fajl/dir..
	short major;          // Major device number (T_DEV only) blize opisuje uredjaj
	short minor;          // Minor device number (T_DEV only)
	short nlink;          // Number of links to inode in file system shortcut fajla, koliko imamo precica
	uint size;            // Size of file (bytes) velicina dinode u bajtovima
	uint addrs[NDIRECT+2];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
	ushort inum;
	char name[DIRSIZ];
};

extern int showEcho;
extern int secretKey;