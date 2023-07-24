#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"


void
printHelp(){

    printf("\nUse this program to set the current active key.\n"
    "After setting the key, you can encr and decr with that key\n"
    "Usage: setkey [OPTION]...[KEY]\n\n"
    "Command line options:\n"
    "       -h, --help: Show help prompt.\n"
    "       -s, --secret: Enter the key via STDIN. Hide key when enteing it.\n");


}

int isdigit(int c) { return '0' <= c && c <= '9'; }

int
main(int argc, char *argv[])
{   

    if(argc==1 || argc>2){
        printf("setkey: command not found\n");
        exit();
    }

    if(!strcmp("-h", argv[1]) || !strcmp("--help", argv[1])){
        printHelp();
        exit();
    }
    
    if(!strcmp("-s", argv[1]) || !strcmp("--secret", argv[1])){
        int status;
        printf("Enter the key: ");

        status = setecho(0);
        if(status==-1){printf("setkey: system error while setting echo\n"); exit();} 

        char* key = "";
        gets(key, 60);
        
        status = setecho(1);  
        if(status==-1){printf("setkey: system error while setting echo\n"); exit();} 

        if(!isdigit(*key)){
            printf("setkey: please enter 0 or any other positive number\n");
            exit();
        }

        int a = atoi(key);
        status = setkey(a);

        if(status==-1) printf("setkey: system error while setting key\n");

        exit();
    }

    if(isdigit(*argv[1])){
        int a = atoi(argv[1]);
        setkey(a);
        exit();
    }else{
        printf("setkey: please enter 0 or any other positive number\n");
        exit();
    }
}
