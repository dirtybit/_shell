#include <stdio.h>
#include <stdlib.h>

#include <readline/readline.h>
#include "linkedlist.c"

#include <unistd.h>
#include <linux/limits.h>

#define PROMPT "myshell"

char **get_cmd_params(list *cmd, char **in, char **out, char **err)
{
	char **arg_arr = (char **) malloc(sizeof(char *)*(cmd->size+1));
	list_node *cur_tok = cmd->begin;
	int i = 0;

	*in = *out = *err = NULL;

	while (cur_tok) {
		if (!strcmp(cur_tok->value, "1>")) {
			*out = cur_tok->next->value;
			cur_tok = cur_tok->next;
		} else if (!strcmp(cur_tok->value, "2>")) {
			*err = cur_tok->next->value;
			cur_tok = cur_tok->next;
		} else if (!strcmp(cur_tok->value, "<")) {
			*in = cur_tok->next->value;
			cur_tok = cur_tok->next;
		} else
			arg_arr[i++] = cur_tok->value;
		cur_tok = cur_tok->next;
	}
	arg_arr[i] = NULL;

	return arg_arr;
}

int main(int argc, char *argv[])
{
	char *prompt;
	char *line;
	char *token;
	char **arg_arr;
	list *cmds, *cmd;
	list_node *cur_cmd;
	char *in, *out, *err;
	char *cwd;
	int cpid, status;
	int **pipefds;

	cmds = list_init();
	cwd = (char *) malloc(sizeof(char)*PATH_MAX);
	prompt = (char *) malloc(sizeof(char)*(PATH_MAX+strlen(PROMPT)+4));
	pipe_buf = (char *) malloc(sizeof(char)*P_BUF_SIZE);

	getcwd(cwd, PATH_MAX);
	sprintf(prompt, "%s:%s$ ", PROMPT, cwd);

	while (1) {
		line = readline(prompt);
		token = strtok(line, " ");

		if (!token)
			continue;
		else if (strcmp(token, "cd") == 0) {
			if (!chdir(strtok(NULL, " "))) {
				getcwd(cwd, PATH_MAX);
				sprintf(prompt, "%s:%s$ ", PROMPT, cwd);
			}
			free(line);
			continue;
		} else if (strcmp(token, "exit") == 0)
			exit(0);
		else {
			cmd = list_init();
			list_add(cmds, cmd);
			list_add(cmd, token);
		}

		/* tokenize command and construct list of commands */
		while (token = strtok(NULL, " ")) {
			if (strcmp(token, "|") == 0) { /* new cmd */
				cmd = list_init();
				list_add(cmds, cmd);
				continue;
			} else {/* next token of current command */
				list_add(cmd, token);
			}
		}

		if (cmds->size == 1) { /* no pipe, simple command run */
			cpid = fork();
			if (!cpid) {
				arg_arr = get_cmd_params(cmds->begin->value, &in, &out, &err);
				execvp(arg_arr[0], arg_arr);
				printf("Error: Command not found\n");
				_exit(0);
			}
		} else {/* piped commands */
			int i = 0;
			pipefds = (int **) malloc(sizeof(int *)*(cmds->size-1));
			for (i = 0; i < cmds->size-1; i++) {
				pipefds[i] = (int *) malloc(sizeof(int)*2);
				pipe(pipefds[i]);
			}

			for (i = 0, cur_cmd = cmds->begin; cur_cmd; cur_cmd = cur_cmd->next, i++) {
				cpid = fork();
				if (cpid == -1) { /* Failure */
				} else if (cpid == 0) { /* Child */
					arg_arr = get_cmd_params(cur_cmd->value, &in, &out, &err);

					if (i == 0) /* first command */
						dup2(pipefds[i][1], 1);
					else if (i == cmds->size-1) /* last command */
						dup2(pipefds[i-1][0], 0);
					else if (i < cmds->size-1) { /* any command in the middle */
						dup2(pipefds[i-1][0], 0);
						dup2(pipefds[i][1], 1);
					}

					for (i = 0; i < cmds->size-1; i++) {
						close(pipefds[i][0]);
						close(pipefds[i][1]);
					}

					execvp(arg_arr[0], arg_arr);
					_exit(0);
				}
			}

			/* Only parent gets here */
			for (i = 0; i < cmds->size-1; i++) {
				close(pipefds[i][0]);
				close(pipefds[i][1]);
			}
		}

		while (wait(0) > 0)
			;

		/* don't forget to free lists */
		for (cur_cmd = cmds->begin; cur_cmd; cur_cmd = cur_cmd->next)
			list_free(cur_cmd->value);
		list_empty(cmds);
		free(line);
	}
	list_free(cmds);

	return 0;
}
