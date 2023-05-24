#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LEN 100
#define MAX_COMMANDS 10
#define MAX_COMMAND_LEN 20
#define MAX_PATH_LEN 100
#define FINISH_PIPE 1
#define IS_PIPE 1

// COMMANDS TO BE ACCEPTED:
// ls - y
// cat - y
// grep - y
// cd - y
// mkdir - y
// rmdir - y
// exit - y
// 
// FUNCTIONALITY:
// piping - y
// show the current working directory - y
// take any number of spaces, like regular UNIX shell - y
// no repeating ';' - y
// reject invalid commands - y
// add support for && and || - n
// add support for ~ for HOME and ~asdf as an alias for the HOME directory of user asdf - n
// search the PATH for incomplete path and execute the given path (relative/absolute) - n
//
// REIMPLEMENTATION IDEA:
// use spans (pointers from start to finish of line)
// minimized copying of strings

int make_pchild(char *const arg[], int *p, int is_pipe) {
	int ret = fork();
	if (ret == 0) {
		if (is_pipe) {
			if (dup2(p[1], STDOUT_FILENO) < 0)
				err(-1, "failed to dup");
		
			if (close(p[0]))
				err(-1, "failed to close p[0]");
		
			if (close(p[1]))
				err(-1, "failed to close p[1]");

		}
		if (execvp(arg[0], arg) < 0)
			err(-1, "failed to exec");
	} else if (ret > 0) {
		ret = 0;
		if (is_pipe) {
			if (dup2(p[0], STDIN_FILENO) < 0)
				err(-1, "failed to dup");
			if (close(p[1]))
				err(-1, "failed to close p[1]");
			
			if (close(p[0]))
				err(-1, "failed to close fd_other");

			p[0] = 0;
			p[1] = 0;
			if (pipe(p) < 0)
				err(-1, "failed to pipe");
		}
		waitpid(ret, NULL, WCONTINUED);
	}
	return ret;
}

int parse(char commands[MAX_COMMANDS][MAX_COMMAND_LEN], char *line) {
	int i, com_num = 0, com_i = 0, spaces = 0, pipes = 0, semi_c = 0, len = 0;

	for (i = 0; i < MAX_LEN; i++) {
		if (line[i] == '\n') {
			commands[com_num][com_i] = '\0';
			break;
		} else if (line[i] == ' ') {
			spaces++;
			len = 0;
			
			if (spaces == 1 && commands[com_num][0] != ';' && com_i != 0) {
				commands[com_num][com_i] = '\0';
				if (com_num + 1 < MAX_COMMANDS)
					com_num++;
				else
					break;
				com_i = 0;
			}
			continue;
		} else if (line[i] == ';') {
			semi_c++;
			if (semi_c != 1 || (com_i == 0 && com_num == 0))
				errx(1, "Unexpected token: %c", line[i]);
			
			if (com_i != 0) {
				commands[com_num][com_i] = '\0';
				if (com_num + 1 < MAX_COMMANDS)
					com_num++;
				else
					break;
				com_i = 0;
			}
			
			len = 0;
			spaces = 0;
			strcpy(commands[com_num], ";\0");
			if (com_num + 1 < MAX_COMMANDS)
				com_num++;
			else
				break;
			continue;
		}

		if (line[i] == '|') {
			pipes++;
			if (pipes > 1)
				continue;
			if (i - 1 >= 0 && line[i - 1] != ' ') {
				com_i = 0;
				if (com_num + 1 < MAX_COMMANDS) {
					com_num++;
					commands[com_num][com_i] = line[i];
					continue;
				} else
					break;
			} else if (i + 1 < MAX_LEN && line[i + 1] != ' ') {
				com_i = 0;
				if (com_num + 1 < MAX_COMMANDS) {
					com_num++;
					commands[com_num][com_i] = line[i];
					continue;
				} else
					break;
			}
		} else
			pipes = 0;

		spaces = 0;
		semi_c = 0;

		if (++len >= MAX_COMMAND_LEN) {
			commands[com_num][com_i] = '\0';
			
			if (len == MAX_COMMAND_LEN && com_num + 1 < MAX_COMMANDS) {
				com_num++;
				com_i = 0;
			} else
				break;
			
			continue;
		}

		commands[com_num][com_i] = line[i];
		com_i++;
	}
	return 0;
}

int main() {
	char pwd[MAX_PATH_LEN];
	char buff[MAX_LEN];
	char commands_buff[MAX_COMMANDS][MAX_COMMAND_LEN];
	char * args[MAX_COMMANDS];
	int i, j, k, ret;
	int *p = calloc(2, sizeof(int));
	char readbuf[MAX_PATH_LEN];
	int save_stdin;

	if ((save_stdin = dup(STDIN_FILENO)) < 0)
		err(1, "failed to dup");
	
	while (1) {
		if (dup2(save_stdin, STDIN_FILENO) < 0)
			err(1, "filed to dup");

		if (getcwd(pwd, MAX_PATH_LEN) == NULL)
			err(-1, "getcwd failed");
		
		for (i = 0; i < MAX_PATH_LEN; i++) {
			if (pwd[i] == '\0') {
				pwd[i] = ' ';
				pwd[i + 1] = '>';
				pwd[i + 2] = '>';
				pwd[i + 3] = '>';
				pwd[i + 4] = ' ';
				pwd[i + 5] = '\0';
				break;
			}
		}

		for (k = 0; k < MAX_COMMANDS; k++) {
			args[k] = NULL;
			commands_buff[k][0] = '\0';
		}
		
		printf("%s", pwd);
		fflush(stdout);
		
		if (fgets(buff, MAX_LEN, stdin) == NULL)
			err(1, "Nothing read from stdin\n");
		
		if (parse(commands_buff, buff) != 0)
			err(1, "Parsing user string failed\n");

		for (i = 0; i < MAX_COMMANDS; i++) {
			if (*commands_buff[i] == '\0')
				break;
			
			if (!memcmp(commands_buff[i], "ls", 2) || !memcmp(commands_buff[i], "cat", 3) || !memcmp(commands_buff[i], "mkdir", 5) || !memcmp(commands_buff[i], "rmdir", 5) || !memcmp(commands_buff[i], "grep", 4)) {
				for(j = 0; (j + i) <= MAX_COMMANDS; j++) {
					if (j == 0) {
						if (!memcmp(commands_buff[i], "ls", 2))
							args[j] = "/usr/bin/ls";
						else if (!memcmp(commands_buff[i], "cat", 3))
							args[j] = "/usr/bin/cat";
						else if (!memcmp(commands_buff[i], "mkdir", 5))
							args[j] = "/usr/bin/mkdir";
						else if (!memcmp(commands_buff[i], "rmdir", 5))
							args[j] = "/usr/bin/rmdir";
						else if (!memcmp(commands_buff[i], "grep", 4))
							args[j] = "/usr/bin/grep";
						continue;
					}
					
					if (*commands_buff[j + i] != ';' && *commands_buff[j + i] != '\0') {
						if (*commands_buff[j + i] == '|') {
							if (pipe(p) < 0)
								err(1, "Unable to make pipe in write");
							
							ret = make_pchild(args, p, IS_PIPE);
							if (ret < 0)
								err(1, "failed to fork");
							
							for (k = 0; k <= j; k++)
								args[k] = NULL;
							
							i = i + j;
							
							break;
						}
						
						args[j] = commands_buff[j + i];
					} else {
						ret = make_pchild(args, p, !IS_PIPE);
						
						for (k = 0; k <= j; k++)
							args[k] = NULL;
						if (ret < 0)
							err(1, "failed to fork");

						i = i + j - 1;
						break;
					}
				}
			} else if (!memcmp(commands_buff[i], "exit", 4)) 
				return 0;
			else if (!memcmp(commands_buff[i], "cd", 2)) {
				if (i + 1 < MAX_COMMANDS && *commands_buff[i + 1] != ';' && *commands_buff[i + 1] != '\0') {
					chdir(commands_buff[i + 1]);
					i++;
				} else
					printf("Missing destination dir...\n");
			} else if (*commands_buff[i] != ';')
				printf("Invalid command: %s\n", commands_buff[i]);
			
		}
	}

	if(close(save_stdin))
		err(1, "failed to close saved stdin");

	return 0;
}
