//
// Created by Robert Jackson on 5/18/2016.
//
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/wait.h>

#define ARYSZ(x)  ( (sizeof(x) / sizeof((x)[0])) )

//*********************DECLARATIONS
//FILE * open_file(char * dir, char *int  get_room_type(char * file); //gets the type of room
//void get_cmd(char * c, char * a);  // getting user input

//*********************VARIABLES
const int  CMDSIZE = 2048; // max size of commands
const int ARGSIZE = 512; // max size of args
char *args;
char *cmd;
int status;

//**********************Functions
size_t array_byteCount(char * arr){
    int pos = 0;

    while(arr[pos] != '\0'){
        pos++;
    }
    return (size_t)pos;
}

//error function
int error(char *msg)
{
    perror(msg);
    return 1;
}

//#################### Get Direction of the user
// sets the ans variable for use in other functions

void get_cmd(char **c, char **a){

    size_t totalSize = CMDSIZE+ARGSIZE;
    unsigned long index =0;
    char *ans;

    ans = (char *)malloc(totalSize+1); // +1 null term
    memset(ans, 0, totalSize); // set to 0

    printf("\nMARCEL-0.1:> ");
    fgets(ans, (int)(totalSize), stdin); // read form stdin

    if(!strchr(ans, '\n'))
        while(fgetc(stdin)!='\n');//discard until newline

    ans[strcspn(ans, "\n")] = 0; //get rid of the \n char at the end

    //get the first occurance of a space so we can separate cmd from args
    char * ptrFO = strchr(ans, ' ');
    index = ptrFO ? ptrFO - ans : 0;

    if (index != 0){
        // set up the cmd
        *c = malloc(sizeof(char) * index+1); //
        strncpy(*c, ans, index);

        // set up the args
        size_t argSize = array_byteCount(ans)-index;
        *a = malloc(sizeof(char) * argSize+1);
        strcpy(*a, ans+index+1);

    }else{
        //just the command
        *c = malloc(sizeof(char) * array_byteCount(ans)+1); //
        strncpy(*c, ans, array_byteCount(ans));

        //set args to Null
        *a = malloc(sizeof(char) * 1); //

    }

//    printf("your args are %s \n" , *a);
//    printf("your command was: %s \n" , *c);
//    printf("your entire was: %s \n" , *a);

    free(ans);

}

exec_cmd(char *c, char *a){

    //if cmd equals shell command
    if(strcmp(c , "exit") == 0){

        exit(0);

        //Changing the Dir
    }else if(strcmp(c , "cd") == 0){

        if(a == NULL){
            a = realloc(a, sizeof(char*)*2);
            a[0]='~';
        }
        //change the directory, and if failed putout stderror
        if(chdir(a) !=0){
            fprintf(stderr, "Encountered an error changing directories\n");
        };
    // print out status
    }else if(strcmp(c , "status") == 0){
        if(status == 0){

            printf("%i", status);
        }else{

            fprintf(stderr, "%i", status);
        }
    }else{

        exec_inShell(c, a);

    }
}

int exec_inShell(char * c, char * a){

    pid_t pid, wpid;

    char * PATH = getenv("PATH");
    printf("%s", PATH);
    pid = fork();

    switch((int)pid){

        case 0:

           //child process
            if(execvp(c, a) < 0){

                status = error("bam!");
            }
            return status;
        case -1:

            status = error("boom!");
            return status;

        default:
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            return 0;
    }

}

main(int argc, char *argv[]){


//
//
        char * command =NULL;
        char * argus =NULL;

        get_cmd(&command, &argus);
         exec_cmd(command, argus);

    //printf("your args are %s \n" , argus);
   // printf("your command was: %s \n" , command);

    fflush;
        free(argus);
        free(command);


}