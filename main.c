#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>

int forknife_cd(char **args);
int forknife_path(char **args);
int forknife_exit(char **args);
char path[512] = "/bin";

//built-in commands are placed into the char array 'builtin_str[]'
//the pointer function 'builtin_func[]' contains the commands and corresponding functions
char *builtin_str[] ={ "cd", "path", "exit" };
int (*builtin_func[])(char **) = {
	&forknife_cd,
	&forknife_path,
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
//this is a built-in command to change the path
int forknife_path(char **args){
	int counter = 0;
	char wholename[512];
	char *next_piece;
	if (args[1] == NULL){
		fprintf(stderr, "forknife: expected argument to \"path\"\n");
	}
	else{
		char *trunc_args;
		trunc_args = args[1];
		strncpy(path, trunc_args, 511);
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
	int found = 0;
	char wholename[512];
	char *next_piece;
	int execute_found = 0;
	printf("path variable currently contains: %s\n", path);
	next_piece = strtok(path, ":");
	printf("next directory to check with 'access()': %s\n", next_piece);
	
	snprintf(wholename, 511, "%s/%s", next_piece, args[0]);//try *args[0], args, *args
	printf("Looking for: %s\n", wholename);
	if(access(wholename, X_OK) == 0) {
		printf("YAY, found executable in: %s\n", wholename);
		found = 1;//found executable
	}
	else {
		while(next_piece != NULL) {
			next_piece = strtok(NULL, ":");
			printf("next directory to check with 'access()': %s\n", next_piece);
			snprintf(wholename, 511, "%s/%s", next_piece, args[0]);//try *args[0], args, *args
			printf("Looking for: %s\n", wholename);
			if(access(wholename, X_OK) == 0) {
				printf("YAY, found executable in: %s\n", wholename);
				found = 1;//found executable
				break;
			}
		}
	}
		
/*	
	snprintf(path_token, 511, "%s/%s", &path[0], *args);
	if(access(path_token, X_OK) == 0) {	
		execute_found = 1;
		printf("yey, you're good!\n");
	}
	else {
		while (access(path_token, X_OK) != 0) {//while can't find executable
			snprintf(path_token, 511, "%s/%s", &path[counter], *args);//check next directive
			if(access(path_token, X_OK) == 0) {	
				execute_found = 1;
				printf("yey, you're good!\n");
				break;
			}
			counter++;
		}
	}
*/
//	if(execute_found) {
//		printf("2nd time: yey, you're good!\n");
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
/*	}
	else {
		perror("forknife");
		return 1;
	}
*/
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
		if(strcmp(args[0], builtin_str[i]) == 0) {//if the command is built-in
			return (*builtin_func[i])(args);
		}
	}
	return forknife_launch(args);//otherwise, execv
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

	token = strtok(line, FORKNIFE_TOK_TOK_DELIM);
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

	token = strtok(NULL, FORKNIFE_TOK_TOK_DELIM);
	
	}//while
	tokens[position] = NULL;
	return tokens;
}//splitline

void forknife_loop(void) {
	char *line;
	char **args;
	int status;

	do{
		printf("forknife>");
		line = forknife_read_line();
		args = forknife_split_line(line);
		status = forknife_execute(args);

		free(line);
		free(args);

	}while (status);

}//forknife_loop

int main(int argc, char **argv) {

	forknife_loop();

	return EXIT_SUCCESS;

}

