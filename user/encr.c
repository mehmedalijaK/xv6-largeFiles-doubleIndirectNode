#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void
printHelp(){
    printf("\nUse this program to encrypt files written on the disk.\n"
    "Usage: encr [OPTION]...[FILE]...\n\n"
    "Command line options:\n"
    "       -h, --help: Show help prompt.\n"
    "       -a, --encrypt-all: Encrypt all files in CWD with current key.\n");
}

char*
fmtname(char *path)
{
	static char buf[DIRSIZ+1];
	char *p;

	// Find first character after last slash.
	for(p=path+strlen(path); p >= path && *p != '/'; p--)
		;
	p++;

	// Return blank-padded name.
	if(strlen(p) >= DIRSIZ)
		return p;
	memmove(buf, p, strlen(p));
	memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
	return buf;
}

void
encrypt(char *path){

	int fd;
	struct stat st;

	if((fd = open(path, O_RDWR)) < 0){
		fprintf(2, "encr: cannot open %s\n", path);
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "encr: cannot stat %s\n", path);
		close(fd);
		return;
	}

    int status = encr(fd);

    switch(status){
        case -1:
            printf("encr: there is no key\n");
            break;
        case -2:
            printf("encr: file is T_DEV type\n");
            break;
        case -3:    
            printf("encr: file is already encrypted\n");
            break;
        case -4:
            printf("encr: fd key error\n");
            break;
        case -5:
            printf("encr: file error\n");
        case 0:
            printf("encr: done\n");
            break;
        default:
            printf("encr: error\n");
    }
	
    close(fd);
}

void
encrAll(){
    
    char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if((fd = open(".", 0)) < 0){
		fprintf(2, "encr: cannot open %s\n", ".");
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "encr: cannot stat %s\n", ".");
		close(fd);
		return;
	}

    if(strlen(".") + 1 + DIRSIZ + 1 > sizeof buf){
        printf("encr: path too long\n");
        exit();
    }

    strcpy(buf, ".");
    p = buf+strlen(buf);
    *p++ = '/';

    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
            printf("encr: cannot stat %s\n", buf);
            continue;
        }

        if(st.type == 2)  {
            encrypt(buf);
        }
              
    }

    close(fd);
}


int isdigit(int c) { return '0' <= c && c <= '9'; }

int
main(int argc, char *argv[])
{

    if(argc==1){ 
        printHelp();
        exit(); 
    }

    if(argc > 2){
        printf("encr: command not found\n");
        exit();
    }

    if(!strcmp("-h", argv[1]) || !strcmp("--help", argv[1])){
        printHelp();
        exit();
    }
    else if(!strcmp("-a", argv[1]) || !strcmp("--encrypt-all", argv[1])){
        encrAll();
        exit();
    }
    else{
        encrypt(argv[1]);
        exit();
    }
}
