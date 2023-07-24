struct file {
	enum { FD_NONE, FD_PIPE, FD_INODE } type;
	int ref; // reference count -> koliko ga procesa koristi
	char readable; // da li je za citanje
	char writable; // da li je za pisanje
	struct pipe *pipe;
	struct inode *ip; //veze se za svoj inode
	uint off; // offset kada stignemo do neke tacke da nastavimo gdje smo stali
};


// in-memory copy of an inode
struct inode {
	uint dev;           // Device number
	uint inum;          // Inode number
	int ref;            // Reference count
	struct sleeplock lock; // protects everything below here
	int valid;          // inode has been read from disk? // da li je sa diska ili sa kesa

	short type;         // copy of disk inode 
	short major;
	short minor;
	short nlink;
	uint size;
	uint addrs[NDIRECT+2]; //prosirujemo za jos jedan double indirect block
};

// table mapping major device number to
// device functions
struct devsw {
	int (*read)(struct inode*, char*, int);
	int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1
