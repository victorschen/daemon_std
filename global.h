/*		
**	global.h    include the global variables.
*/
#ifndef _GLOBLE_H_
#define _GLOBLE_H_

/*  flag is used to differentiate the level 5's recovery and resync!
**   0: new create
**   1: normal
**   2: recovery
*/
//int flag[32];

int mb = 0;
char workingDir[] = "/usr/local/apache/htdocs/RaidManager/Monitor/"; 
char cdir[] = "/home/c/";
char ldir[] = "/home/l/";
char edir0[] = "/home/e0/";
char edir1[] = "/home/e1/";
char udir[] = "/home/u/";
char odir[] = "/home/o";
char bdir[] = "/home/b";
char daemondir[] = "/usr/local/apache/htdocs/RaidManager/Compile/";

#endif
