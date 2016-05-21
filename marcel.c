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
#include "marcel.h"
#include <sys/types.h>


#define ARYSZ(x)  ( (sizeof(x) / sizeof((x)[0])) )

//*********************DECLARATIONS
//FILE * open_file(char * dir, char *int  get_room_type(char * file); //gets the type of room
//void get_cmd(char * c, char * a);  // getting user input

//*********************VARIABLES
const int  CMDSIZE = 2048; // max size of commands
const int ARGSIZE = 512; // max size of args
const int MAXARGS = 2560;
char *cmd;
int status;

//**********************Functions


//error function
int error(char *msg)
{
    perror(msg);
    return 1;
}

//#################### Get Direction of the user
// sets the ans variable for use in other functions

char** get_cmd(){

    //todo find a better way to deal with the buffer sizes here - seriously way too big.
    int count =0;
    size_t totalSize = CMDSIZE+ARGSIZE;
    unsigned long index =0;
    char *ans, *toke;
    char **args = malloc(sizeof(char*) * totalSize);

    ans = (char *)malloc(totalSize+1); //hold the user unput

    //output to screen the prompt
    printf("\nMARCEL-0.1:> ");
    fgets(ans, (int)(totalSize), stdin); // read form stdin

    if(!strchr(ans, '\n'))
        while(fgetc(stdin)!='\n');//discard until newline

    //get rid of the \n char at the end
    ans[strcspn(ans, "\n")] = 0;

    //put the rest of the args in the remaining postions
    toke = strtok(ans, " ");
    int pos = 0;
    while (toke != NULL){

        args[pos] = toke;
        toke = strtok(NULL, " "); //update toke, to next space
        pos++;
    }

    args = realloc(args, sizeof(char *) * pos); // trim off what we dont need;
    free(ans);
    free(toke);

    return args;


}

int exec_cmd(char **cmd){

    // First, handle Built-ins
    //if cmd equals shell command
    if(strcmp(cmd[0] , "exit") == 0){

        exit(0);

    //Changing the Dir
    }else if(strcmp(cmd[0] , "cd") == 0){

        if(ARYSZ(cmd) <= 1){
            cmd = realloc(cmd, sizeof(char*)*2);
            cmd[1]= "~";
        }
        //change the directory, and if failed putout stderror
        if(chdir(cmd[1]) !=0){
            error( "Encountered an error changing directories\n");
            return 0;
        };
    // print out status
    }else if(strcmp(cmd[0] , "status") == 0){

            printf("%i", status);
            return 0;

    }else{

        // not a built-in? Execute it.
        return exec_inShell(cmd);
    }

}


int exec_inShell(char ** cmd){

    pid_t pcessID = -5;
    pid_t wpid;
//    char *args[1];
//    *args = cmdline; // exec takes a weird argument here


    const char * PATH = getenv("PATH");

    pcessID = fork();
    printf("spawning processes..%i", pcessID);
    switch((int)pcessID){

        case -1:
            //it's in a bad state
            status = error("boom!");
            return status;

        case 0:

           //child process
            status = execvp(cmd[0], cmd);

            //unless exec fails  this never executes
            error("bam!");
            return status;

        default:
            do {
                wpid = waitpid(pcessID, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status) );

//            if(WIFSTOPPED(status)){
//                error("Child process was stopped");
//                printf("status stop signal was:%i", WSTOPSIG(status));
//            }
            printf("process id that completed is %i", (int)wpid);
            return status;
    }
}

int main(int argc, char *argv[]){

    while(status == 0){

        char ** cmd = NULL;

        cmd = get_cmd(); // get the command from user
        exec_cmd(cmd); // exec on it

    }

}