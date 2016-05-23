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
int BIStatus = 0;

typedef struct kiddos{
    pid_t * childProcs;
    int size;
    int cap;
}kiddos;

//error function
int error(char *msg)
{
    perror(msg);
    return 1;
}


//Struct Array functions - adapted from CS261, 2014
void _initKiddos(kiddos *v, int capacity)
{
    assert(capacity > 0);
    assert(v!= 0);
    v->childProcs = malloc(sizeof(pid_t) * capacity);
    assert(v->childProcs != 0);
    v->size = 0;
    v->cap = capacity;

}

kiddos* createKiddos(int cap)
{
    kiddos *r;
    assert(cap > 0);
    r = malloc(sizeof( kiddos));
    assert(r != 0);
    _initKiddos(r,cap);
    return r;
}

/* Deallocate data array in Kiddos.

	param: 	v		pointer to the dynamic array
	pre:    v is not null
	post:	d.data points to null
	post:	size and capacity are 0
	post:	the memory used by v->data is freed
*/
void freeKiddos(kiddos *v)
{
    assert(v!=0);

    if(v->childProcs != 0)
    {
        free(v->childProcs); 	/* free the space on the heap */
        v->childProcs = 0;   	/* make it point to null */
    }
    v->size = 0;
    v->cap = 0;
}

/* Deallocate data array and the dynamic array ure.

	param: 	v		pointer to the dynamic array
	pre:	v is not null
	post:	the memory used by v->data is freed
	post:	the memory used by d is freed
*/
void deleteKiddos(kiddos *v)
{
    assert (v!= 0);
    freeKiddos(v);
    free(v);
}

int open_file(char * fileName, int rw){

    int fd;

    //open file for reading unless flag is set
    fd = open(fileName, O_RDONLY);
    if (rw == 1){
        fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    }

    if(fd < 0) { // opening fails

        error("Couldn't open file " );
        BIStatus =1;
        return 1;
    }

    return fd; // returning the file pointer
}

size_t addChild(kiddos* kids, pid_t id){

    kids->childProcs[kids->size]= id;
    kids->size++;

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


//TODO Alter this so that it returns-maybe the number of redirs?
//make a separate to determine positions
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


int checkBackground(char ** cmd){
    int pos = 0;

    while(cmd[pos] != NULL){

        if((strcmp(cmd[pos], "&") == 0)){
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
            cmd[1]= getenv("HOME");

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

    //we have a comment ignore it
    }else if((strncmp(cmd[0] , "#", 1) == 0)){

        return 0;

    } else{

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
            status = 1;
            error("boom!");
            return status;

        case 0:


            //if process has a redirect:
            if ((rpos = checkRedirect(cmd)) > 0){

                fd = changeOut(cmd, rpos);  //alter the redirects as needed

                //strip out the end of cmd we wont need it anymore
                cmd[rpos] = NULL;
                cmd = realloc(cmd, (size_t)rpos);

                //REQ print out if file cannot be created
                if(fd > 0){

                    error("Could not redirect");
                    exit(1);
                }
            }

            //If process has a &
            if(rpos = checkBackground(cmd)){

            }


           //child process execute
            status = execvp(cmd[0], cmd);

            //it failed
            if(status < 0){

                //kill it!!
                fprintf(stdout, "Child process quit with status: %i", status);
                //unless exec fails  this never executes
                error("bam!");
                BIStatus = 1;
                return status;

            }else{ //child did something  well, praise it

                //there was a redirect and it finished;, we need to close a file
                if(fd > 0){
                    test=  close(fd);
                    printf("%i", test);
                }

                return status;
            }




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

            fprintf(stdout, "Child process id that completed is %i\n", (int)wpid);
            fprintf(stdout, "Child process exit status %i\n", status);
            return status;
    }
}

int main(int argc, char *argv[]){
    //Initialzations:
    kiddos* kids;
    kids = createKiddos(100);

    while(status == 0){

        char ** cmd = NULL;
        cmd = get_cmd(); // get the command from user

        if(cmd[0] != NULL){ // it's not blank
            status = exec_cmd(cmd); // exec on it
        }


       // printf("Redirect is %i, cmdSize is %i", checkRedirect(cmd), checkCmdSize(cmd) );
    }
    free(cmd);


//    //we'll use this and others to store the kids, and check them delete them etc.
//     addChild(kids, getpid()) ;
//    int i = 0;
//    for(i=0; i < kids->size; i++ ){
//        printf("%i", (int)kids->childProcs[i]);
//    }



}