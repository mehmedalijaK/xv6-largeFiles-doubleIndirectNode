#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void
printHelp(){
    printf("\nUse this program to decrypt files written on the disk.\n"
    "Usage: decr [OPTION]...[FILE]...\n\n"
    "Command line options:\n"
    "       -h, --help: Show help prompt.\n"
    "       -a, --decrypt-all: Decrypt all files in CWD with current key.\n");
}

int isdigit(int c) { return '0' <= c && c <= '9'; }

void
decrypt(char* path){

	int fd;
	struct stat st;

	if((fd = open(path, O_RDWR)) < 0){
		fprintf(2, "decr: cannot open %s\n", path);
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "decr: cannot stat %s\n", path);
		close(fd);
		return;
	}

    int status = decr(fd);

        switch(status){
        case -1:
            printf("decr: there is no key\n");
            break;
        case -2:
            printf("decr: file is T_DEV type\n");
            break;
        case -3:    
            printf("decr: file is not encrypted\n");
            break;
        case -4:
            printf("decr: fd key error\n");
            break;
        case -5:
            printf("decr: file error\n");
        case 0:
            printf("decr: done\n");
            break;
        default:
            printf("decr: error\n");
    }

    close(fd);
}

void
decrAll(){
    
    char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if((fd = open(".", 0)) < 0){
		fprintf(2, "decr: cannot open %s\n", ".");
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "decr: cannot stat %s\n", ".");
		close(fd);
		return;
	}

    if(strlen(".") + 1 + DIRSIZ + 1 > sizeof buf){
        printf("decr: path too long\n");
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
            decrypt(buf);
        }
              
    }

    close(fd);
}

int
main(int argc, char *argv[])
{
    if(argc==1){
        printHelp();
        exit();
    }

    if(argc > 2){
        printf("decr: command not found\n");
        exit();
    }

    if(!strcmp("-h", argv[1]) || !strcmp("--help", argv[1])){
        printHelp();
        exit();
    }
    else if(!strcmp("-a", argv[1]) || !strcmp("--decrypt-all", argv[1])){
        decrAll();
        exit();
    }
    else{ 
        decrypt(argv[1]);
        exit();
    }
}
