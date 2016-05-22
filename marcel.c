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


//error function
int error(char *msg)
{
    perror(msg);
    return 1;
}

FILE * open_file(char * dir, char * fileName, char * action){

    char *file;
    int size = (int)(strlen(dir) + strlen(fileName) + 2);
    FILE *fp;
    file = malloc(sizeof(char)*size);

    //combine dir name and name of file
    snprintf(file, (size_t)size, "%s/%s", dir, fileName);

    //open the newfile for writing
    // with the ROOM: + Room Name
    if((fp = fopen(file , action )) < 0) { // opening fails

        error("Couldn't open file " );
        return NULL;
    }
    return fp; // returning the file pointer
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
//    char *args[1];
//    *args = cmdline; // exec takes a weird argument here

    //if process has a redirect:
    if ((rpos = checkRedirect(cmd)) > 0){
        FILE *fp;
        int fp2;
        char buff[CMDSIZE];

        //get the file name requestd:
            //its between rpos+1 and next space

        fp = open_file(getcwd(buff, CMDSIZE), "temp.tmp", "w+");

        if (fp < 0 ){
            error("cant create temp buffer file");
            exit(42);
        }

        //depending on the type of redirect we change the value in dup2
        if(strcmp(cmd[rpos], ">") == 0){
            dx = 1;
        }

        fp2 = dup2((int)fp, dx);
        if (fp2 < 0 ){
            error("cant create temp dup2 buffer file");
            exit(42);
        }
    }

    //follow instructions on lecture 12@6:10^^^
    //read the file into stdin i guess and
    //


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
                // kill -kill something??? wpid
//            }
            printf("process id that completed is %i", (int)wpid);
            return status;
    }
}

int main(int argc, char *argv[]){

    while(status == 0){

        char ** cmd = NULL;
        int re;
        cmd = get_cmd(); // get the command from user
        exec_cmd(cmd); // exec on it


       // printf("Redirect is %i, cmdSize is %i", checkRedirect(cmd), checkCmdSize(cmd) );
    }

}