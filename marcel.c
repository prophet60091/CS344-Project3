//
// Created by Robert Jackson on 5/18/2016.
//
#define _POSIX_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "marcel.h"

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
int BGStatus = 0;

typedef struct kiddos{
    pid_t * childProcs;
    int size;
    int cap;
}kiddos;

kiddos* kids;

//error function
int error(char *msg)
{
    perror(msg);
    return 1;
}

//todo move the kiddos function class of functions to their own h and file.
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

void removeAtKiddos(kiddos *v, int idx){
    int i;
    assert(v!= 0);
    assert(idx < v->size);
    assert(idx >= 0);

    //Move all elements up

    for(i = idx; i < v->size-1; i++){
        v->childProcs[i] = v->childProcs[i+1];
    }

    v->size--;

}


//OPENS FILES FOR READING OR WRITING
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
        return fd;
    }

    return fd; // returning the file pointer
}

size_t addChild(kiddos* kids, pid_t id){

    kids->childProcs[kids->size]= id;
    kids->size++;

}

//#################### Read the line
// sets the ans variable for use in other functions
char * read_input(){
    size_t lineSize = CMDSIZE;
    char *ansBuffer = NULL; //hold the user unput

    getline(&ansBuffer, &lineSize, stdin);

    //get rid of the \n char at the end
    ansBuffer[strcspn(ansBuffer, "\n")] = 0;

    return ansBuffer;
}

//####################Process the line
// sets the ans variable for use in other functions
char ** process_line(char * cmd){
    int pos;
    char *toke;
    char ** result = malloc(sizeof(char*) * CMDSIZE);


    //put the rest of the args in the remaining postions
    toke = strtok(cmd, " ");
    pos = 0;
    while (toke != NULL){

        result[pos] = toke;
        toke = strtok(NULL, " "); //update toke, to next space
        pos++;
    }
    //add a null to the end
    result[pos] = NULL;

    return result;
}

//Get the  user input:

char** get_cmd(){

    char * line;
    char ** result;
    //output to screen the prompt
    fprintf(stdout, "\nMARCEL-0.1:>");

    line = read_input();
    result = process_line(line);

    return result;

}


//TODO Alter this so that it returns-maybe the number of redirs?
//Checks if there are any redirects
//@param  pointer to pointers of commands
//returns the postion of them in the command
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

//Checks if if the background operator has been used
//@param  pointer to pointers of commands
//returns the position of it in the command
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
//gets the length of the pointer array of pointers
// /note: (cant find a built in function that does this well)
//@param  pointer to pointers of commands
//returns the length
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
//@params they type of change out, 0(swapping stdin or stout for a file) 1(point to dev/null)
int changeOut(char ** cmd, int rpos, int bgFlag){

    int fd2 = -10;
    int fd = -10;
    int dx = 0;  // if this is 0(stdin) were inputting not outputting

    //get the file name requestd:
    //its between rpos+1 and next space rmbr:getcwd(buff, CMDSIZE),
    //get the  char * ptrFO = strchr(ans, ' ');
    switch(bgFlag){

        case 1:

           fd = open_file("/dev/null", 1);

            if (fd < 0 ){
                error("Couldnt open /dev/null");
                return fd;
            }

            //direct standard out at dev/null
            fd2 = dup2(fd, 1);
            if ((int)fd2 < 0 ){
                error("failed creating dev/null dup2");
                return fd2;
            }
            //direct standard in at dev/null
            fd2 = dup2(fd, 0);
            if ((int)fd2 < 0 ){
                error("failed creating dev/null dup2");
                return fd2;
            }
         break;

        default:
        //depending on the type of redirect we change the value in dup2
        if(strcmp(cmd[rpos], ">") == 0){
            dx = 1;
        }

        fd = open_file(cmd[rpos+1], dx);

        if (fd < 0 ){
            error("failed creating temp file");
            return fd;
        }

        fd2 = dup2(fd, dx);
        if ((int)fd2 < 0 ){
            error("failed creating temp dup2 buffer file");
            return fd;
        }
    }
    return fd;

}

// Takes the users commands and turns them into actions
// @ param pointer to pointers of the command retuned by get_cmd
// Additionally runs the built in functions
// anything else is passed off to be execd in shell
// returnes 0 when successful
int exec_cmd(char **cmd){

    // First, handle Built-ins
    //if cmd equals shell command
    if(strcmp(cmd[0] , "exit") == 0){

        atexit(turnLightsOFF);
        exit(1);//todo fix this it's not right needs to handle kill of children

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

// Executes the command after forking
// @ param pointer to pointers of the command
// waits on foreground processes
// does not wait on background processes
// returns the status of the command after execution
int exec_inShell(char ** cmd){

    pid_t pcessID = -5;
    pid_t wpid;
    int rpos = 0;
    int fd = -1;
    int bgFlag = 0;

    //have to check bg flag before forking, otherwise we loose the state of this:
    rpos = checkBackground(cmd);
    if(rpos > 0) {
        bgFlag = 1;
    }

    pcessID = fork();
    //printf("spawning processes..%i", pcessID);
    //partially adapted from lecture 9 cs344
    switch((int)pcessID){

        case -1:
            //it's in a bad state
            status = 1;
            error("boom!");
            return status;


        // WERE IN THE CHILD PROCESS
        case 0:

            //todo yes code is redundant, but..
            // relies so much on the values getting set it's hard to pack it into one function
            //if process has a redirect:
            //If process has a & background process request
            rpos = checkBackground(cmd);
            if(rpos > 0){

                fd= changeOut(cmd, rpos, 1);

                //strip out the end of cmd we wont need it anymore
                cmd[rpos] = NULL;
                cmd = realloc(cmd, (size_t)rpos);

                //REQ print out if file cannot be created
                if(fd < 0){

                    error("Could not redirect");
//                    BIStatus = 1;
//                    return BIStatus; // we don't want to kill everything because of one bad execution
                    return status = 1;
                }

            }
            // checks if the code has any redirects
            rpos = checkRedirect(cmd);
            if (rpos  > 0){

                fd = changeOut(cmd, rpos, 0);  //alter the redirects as needed

                //strip out the end of cmd we wont need it anymore
                cmd[rpos] = NULL;
                cmd = realloc(cmd, (size_t)rpos);

                //REQ print out if file cannot be created
                if(fd < 0){

                    error("Could not redirect");

                    return status = 1; // we don't want to kill everything because of one bad execution
                }
            }

            // execute it and save the status of the command
            status = execvp(cmd[0], cmd);

            //it failed
            if(status < 0){

                //kill it!!
                fprintf(stdout, "Child process quit with status: %i- Command was %s", status, cmd[0]);
                error("bam!");

                fflush(stdout); // maybe a good idea?
                return status = 1; // we don't want to kill everything because of one bad execution

            }else{
                // Code executed successfully
                //there was a redirect and it finished;, we need to close a file
                if(fd > 0){
                     close(fd);
                }

                //sometimes we get a bad status but still return ok
                return status = 0;
            }

            // WERE IN THE PARENT
        //ADAPTED FROM **http://brennan.io/2015/01/16/write-a-shell-in-c/**//
        //Too elegant to pass up. it works really well
        //  were storing the result of waitpid using WUNTRACED, (reports its status whether stopped or not)
        // as long as the process didn't exit, or receive a signal, so it's waiting util that happens
        // when it does, we know that the child process is complete.
        default:
            //printf("BGFLAG SET  %i", bgFlag);
            if(!bgFlag){// we're not running a bgnd process so start waiting for the child to quit.

                do {
                    wpid = waitpid(pcessID, &status, WUNTRACED);

                } while (!WIFEXITED(status) && !WIFSIGNALED(status));

                // we fisnishd a process and we have a redirect - better close the file;
                // it might need to also be reset.

                //fprintf(stdout, "Child process id that completed is %i\n", (int) wpid);
                //fprintf(stdout, "Child process exit status %i\n", status);


            }else{
                // store the process - WE HAVE A BG PROCESS TO DEAL WITH
                addChild(kids, pcessID);
                fprintf(stdout, "Background process started: %i\n", pcessID);

            }

            //sometimes we get a bad status but still return ok
            // because shell should keep running
            if(status > 0){
                //BIStatus = 1;
                return status = 1;
            }
    }

    return status; //fixes control reaches end of non-void (never executes tho...)
}
// Handle Background Processes
// Called to clean up BG process that did quit and announce this
// Not sure what I want to return atm....
int handleBackground(){

    int status = 0;
    int i;

    //for each child in the background
    for(i=0; i < kids->size; i++){

        pid_t waiter = waitpid(kids->childProcs[i], &status, WNOHANG);

        if (waiter == 0) {
            // do..nothing everything is just fine
            // let the process ride

        } else if (waiter == -1) {
            //error("Nothing to clean up?...or some orther failure");

        // Process is real - evaluate it
        } else {

            // something went wrong with the process
            if(status > 0) {
                // Error
                //fprintf(bigBuff, "Child Process:%i ended with %i status", (int) kids->childProcs[i], status);
                error("Child Process died...probably dysentery, or lupus...nah, it's never lupus");

                //remove it from the watch list
                removeAtKiddos(kids, i);

                //put it out of it's misery
                kill(kids->childProcs[i], SIGKILL);

               BGStatus = 1;
                // something went right with the process but it's done
            }else{

                // Child exited successfully
                //fprintf(stdout, "Child Process:%i ended with %i status", (int)kids->childProcs[i], status);
               BGStatus = 0;

                //remove it from the watch list
                removeAtKiddos(kids, i);

            }
        }
    }

    return status;
}

void lsh_free_args(char **args)
{
  char **iter = args;
  while (*iter != NULL) {
    free(*iter);
    iter++;
  }
  free(args);
}

void turnLightsOFF(void)
{
    handleBackground();
}

int main(int argc, char *argv[]){

    //Initialzations:
    kids = createKiddos(100);

    //If status is 1 or 0 we're good
    // 1 means we got an error from child
    // 0 means all is square
    // anything else we effed something up, Blame it on management.
    while(status == 0 || status == 1){

        char ** cmd = NULL;
        cmd = get_cmd(); // get the command from user

        if((cmd[0] != NULL) && (strcmp(cmd[0], "")) != 0 && (strcmp(cmd[0], "\\n")) != 0 ){ // it's not blank/ seems redundant but eos server no likey just NULL
            status = exec_cmd(cmd); // exec on it
        }

        handleBackground();
        lsh_free_args(cmd);
    }

    //free the memories
    deleteKiddos(kids);


}