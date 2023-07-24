#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "blockwriter.h"

void
printHelp(){

    printf("\nUse this program to create a big file filled with a-z characters.\n"
    "Default filename: %s\n"
    "Default blocks: %d\n"
    "Usage: blockwriter [OPTION]...\n\n"
    "Command line options:\n"
    "       -h, --help: Show help prompt.\n"
    "       -b, --blocks: Number of blocks to write.\n"
    "       -o, --output-file: Set output filename.\n", FILE_NAME, DEF_BLOCKS);

}

void
createFile(int b, char* s){
    int fd;
    
    if(b>MAXFILE){
        printf("blockwriter: you exceed the limit of max file size\n");
        exit();
    }

    fd = open(s, O_CREATE_RDWR);
    if(fd<0){
        printf("blockwriter: could not create new file\n");
        exit();
    }

    for(int i = 0; i<b; i++){
        int status = write(fd, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqr", 512);
        if(status==512) printf("Writing block %d\n", i);
        else printf("Error writing block %d\n", i);
    }
   
    close(fd);
}

int isdigit(int c) { return '0' <= c && c <= '9'; }

int
main(int argc, char *argv[])
{
    switch(argc)
    {
        case 1:
            createFile(DEF_BLOCKS, FILE_NAME);
            exit();
            break;
        case 2:
            if(!strcmp("-h", argv[1]) || !strcmp("--help", argv[1])){
                printHelp();
                exit();
            }
            printf("blockwriter: command not found\n");
            break;
        case 3:
            if(!strcmp("-b", argv[1]) || !strcmp("--blocks", argv[1])){
                if(isdigit(*argv[2])){
                    createFile(atoi(argv[2]), FILE_NAME);
                    exit();
                }
            }
            else if(!strcmp("-o", argv[1]) || !strcmp("--output-file", argv[1])){
                createFile(DEF_BLOCKS, argv[2]);
                exit();
            }
                printf("blockwriter: command not found \n");
                exit();
            
            break;
        case 5:
            if((!strcmp("-b", argv[1]) || !strcmp("--blocks", argv[1])) && (!strcmp("-o", argv[3]) || !strcmp("--output-file", argv[3])) && (isdigit(*argv[2]))){
               createFile(atoi(argv[2]), argv[4]);
               exit();
            }
            if((!strcmp("-o", argv[1]) || !strcmp("--output-file", argv[1])) && (!strcmp("-b", argv[3]) || !strcmp("--blocks", argv[3])) && (isdigit(*argv[4]))){
               createFile(atoi(argv[4]), argv[2]);
               exit();
            }

            printf("blockwriter: command not found \n");
            exit();

            break;
        default:
            printf("blockwriter: command not found \n");
                exit();
    }

	exit();
}