//
// Created by Robert on 5/20/2016.
//

#ifndef PROJECT3_MARCEL_H
#define PROJECT3_MARCEL_H

int exec_inShell( char ** fullCmd);
int exec_cmd(char **cmd);
char **get_cmd();
int error(char *msg);
int checkCmdSize(char ** cmd);
int checkRedirect(char ** cmd);
int open_file( char * fileName, int rw);
int handleBackground();
int checkBackground(char ** cmd);
void turnLightsOFF(void);

#endif //PROJECT3_MARCEL_H
