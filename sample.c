#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
int
main(int argc, char**argv)
{ 
	char *line = NULL;
	size_t linesize = 0;
	size_t linelen;
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("\nDir: %s",cwd);	
	printf("\nHello! Welcome to Forknife! You're about to have a dabbing good time!\n");
	
	while((linelen = getline(&line, &linesize, stdin)) != -1){
		if(strncmp("exit", line, 4) == 0) {
			printf("Goodbye :^) \n");
			exit(0);
		}
		
		fwrite(line, linelen, 1, stdout);
		
		/*//forking child
		pid_t pid = fork();

		if(pid == -1){//forking failed
			printf("\nForking child failed");
			return 0;
		} 
		else if(pid == 0) {//processing typed in command 
			if (execvp(argv[0], argv) < 0){
				printf("\nCould not execute command");
			}
		exit(0);
		}
		else {//waiting for child to terminate 
			wait(NULL);
			return 0;
		}



		//if child, exec
		//if parent, wait
	  */
	  }

	free(line);
	if(ferror(stdin)) {
		err(1,"getline");
	}

}
