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
#include <sys/stat.h>
#include <fcntl.h>

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


//error function
int error(char *msg)
{
    perror(msg);
    return 1;
}

int open_file(char * fileName, int rw){

    int fd;

    //open file for reading unless flag is set
    fd = open(fileName, O_RDONLY);
    if (rw == 1){
        fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, 644);
    }

    if(fd < 0) { // opening fails

        error("Couldn't open file " );
        return -1;
    }

    return fd; // returning the file pointer
}


//#################### Get Direction of the user
// sets the ans variable for use in other functions

char** get_cmd(){

    size_t totalSize = CMDSIZE+ARGSIZE;
    char *ans, *toke;
    char **args = malloc(sizeof(char*) * totalSize);

    ans = (char *)malloc(totalSize+1); //hold the user unput

    //make sure stdin is empty;
    fseek(stdin,0,SEEK_END);

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

    args = realloc(args, sizeof(char *) * pos+1); // trim off what we dont need;
    free(ans);
    free(toke);

    return args;

}

int checkRedirect(char ** cmd){
    int pos = 0;

    while(cmd[pos] != NULL){

        if((strcmp(cmd[pos], "<") == 0) || (strcmp(cmd[pos], ">") == 0)){
            return pos;
        }
        pos++;
    }
    return 0;

}

int checkCmdSize(char ** cmd){
    int pos = 0;
    while(cmd[pos] != NULL){

        pos++;
    }
    return pos;
}

//Changes the output file in the case of redirects
// depending on the direction it switches the output between stdin and stout
//@params array of pointers to char pointers, that contain the issued command
//@params the position of the redirect in the array of pointers
int changeOut(char ** cmd, int rpos){

    int fd2 = -10;
    int fd = -10;
    int dx = 0;  // if this is 0 were inputting not outputting
    char buff[ARGSIZE]; // place to stuff the name used in the redirect

    //get the file name requestd:
    //its between rpos+1 and next space rmbr:getcwd(buff, CMDSIZE),
    //get the  char * ptrFO = strchr(ans, ' ');

    //depending on the type of redirect we change the value in dup2
    if(strcmp(cmd[rpos], ">") == 0){
        dx = 1;
    }

    fd = open_file(cmd[rpos+1], dx);

    if (fd < 0 ){
        error("failed creating temp file");
        exit(42);
    }

    fd2 = dup2(fd, dx);
    if ((int)fd2 < 0 ){
        error("failed creating temp dup2 buffer file");
        exit(42);
    }
    return fd;

}


int exec_cmd(char **cmd){

    // First, handle Built-ins
    //if cmd equals shell command
    if(strcmp(cmd[0] , "exit") == 0){

        exit(0);//todo fix this it's not right

    //Changing the Dir
    }else if(strcmp(cmd[0] , "cd") == 0){

        if(checkCmdSize(cmd) <= 1){
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
    int rpos = 0;
    int dx=0;
    int fd = -1;
    int test;


    pcessID = fork();
    //printf("spawning processes..%i", pcessID);
    //adapted from lecture 9 cs344
    switch((int)pcessID){

        case -1:
            //it's in a bad state
            status = error("boom!");
            return status;

        case 0:

            //if process has a redirect:
            if ((rpos = checkRedirect(cmd)) > 0){

                fd = changeOut(cmd, rpos);

                //strip out the end of cmd we wont need it anymore
                cmd[rpos] = NULL;
                cmd = realloc(cmd, (size_t)rpos);

            }

           //child process execute
            status = execvp(cmd[0], cmd);

            //it failed
            if(status < 0){

                //kill it!!

            }else{ //child did something  well, praise it

                //there was a redirect we need to close a file
                if(fd > 0){
                  test=  close(fd);
                    printf("%i", test);
                }

            }


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
                // kill -kill something??? wpid
//            }
            // we fisnishd a process and we have a redirect - better close the file;
            // it might need to also be reset.

            printf("Child process id that completed is %i\n", (int)wpid);
            printf("Child process exit status %i\n", status);
            return status;
    }
}

int main(int argc, char *argv[]){

    while(status == 0){

        char ** cmd = NULL;
        cmd = get_cmd(); // get the command from user
        status = exec_cmd(cmd); // exec on it

       // printf("Redirect is %i, cmdSize is %i", checkRedirect(cmd), checkCmdSize(cmd) );
    }

}