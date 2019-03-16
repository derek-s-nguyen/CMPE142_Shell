#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int forknife_cd(char **args);
int forknife_exit(char **args);


/*
We are going to have built-in commands in a char array. Then, have a pointer function where the commands will have the corresponding functions.
*/

char *builtin_str[] ={ "cd", "exit"};
int (*builtin_func[])(char **) = {
	&forknife_cd,
	&forknife_exit
};


int forknife_cd(char **args){

if(args[1] == NULL){
fprintf(stderr, "forknife: expected argument to \"cd\"\n");
}

else{
if(chdir(args[1]) != 0){
perror("forknife");
}
}

return 1;
}

//this is a built-in command to exit by returning zero
int forknife_exit(char **args)
{
	return 0;
}
//function that will launch a program and wait for it to terminate
int forknife_launch(char **args)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		//child process
		if(execvp(args[0], args) == -1) {
			perror("forknife");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		//error forking
		perror("forknife");
	} else {
		//parent process
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}
int forknife_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}
//execute built-in command or launch program
int forknife_execute(char **args)
{
	int i;
	if (args[0] == NULL) {
		//an empty command was entered
		return 1;
	}
	for(i = 0; i < forknife_num_builtins(); i++) {
		if(strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}
	return forknife_launch(args);
}

#define FORKNIFE_RL_BUFSIZE 1024
char *forknife_read_line(void)
{
	char *line = NULL;
	ssize_t bufsize = 0;//have getline allocate a buffer for us
	getline(&line, &bufsize, stdin);
	return line;
}

#define FORKNIFE_TOK_BUFSIZE 64
#define FORKNIFE_TOK_TOK_DELIM " \t\r\n\a"

char **forknife_split_line(char *line){
	int bufsize = FORKNIFE_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if(!token){
		fprintf(stderr, "allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strsep(line, FORKNIFE_TOK_TOK_DELIM);
	while(token != NULL){
		tokens[position] = token;
		position++;	

	if(position >= bufsize) {
		bufsize += FORKNIFE_TOK_BUFSIZE;
		tokens = realloc(tokens, bufsize * sizeof(char*));
		if(!tokens) {
			fprintf(stderr, "allocation error\n");
			exit(EXIT_FAILURE);
		}
	}
	
	token = strsep(NULL, FORKNIFE_TOK__TOK_DELIM);
	
	}//while
	tokens[position] = NULL;
	return tokens;
}//splitline

void forknife_loop(void) {
	char *line;
	char **args;
	int status;

	do{
		printf(">");
		line = forknife_read_line();
		args = forknife_split_line(line);
		status = forknife_exectute(args);

		free(line);
		free(args);

	}while (status);

}//forknife_loop

int main(int argc, char **argv) {


	forknife_loop();

	return EXIT_SUCCESS;

}

