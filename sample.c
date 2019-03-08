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

	while((linelen = getline(&line, &linesize, stdin)) != -1){
		if(strncmp("exit", line, 4) == 0) {
			exit(0);
		}
		
		fwrite(line, linelen, 1, stdout):
		//fork
		//if child, exec
		//if parent, wait
	  
	  }
	free(line);
	if(ferror(stdin)) {
		err(1,"getline");
	}

}
