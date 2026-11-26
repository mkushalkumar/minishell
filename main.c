/*documentation :-

Name :- kushal kumar
start date :- 4/03/2026
description :-  The project name is minishell. This project about minishell terminal basic working
                 mechanism as per our few requirments.

*/

#include "main.h"

int pid;
int status;
char cmd[20];
#define MAX_JOBS 100

struct job
{
    pid_t pid;
    char cmd[100];
};
struct job stopped_jobs[MAX_JOBS];
int stopped_count = 0;

int main()
{

    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval", "set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source", "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};

    char *external_commands[MAX_CMDS];
    extract_external_commands(external_commands);

    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, signal_handler);

    char prompt[20] = "minishell$ : ";
    system("clear");

    while (1)
    {
        printf(GREEN "%s", prompt);
        RESET;

        scanf(" %[^\n]", cmd);
        
        char cmd_copy[100];
        strcpy(cmd_copy, cmd);
        char *tmp_cmd = strtok(cmd_copy, " ");
        int type = check_command_type(tmp_cmd, external_commands);

        if (type == 2)
        {

            execute_internal_commands(cmd);
        }
        else if (type == 1)
        {

            pid = fork();

            if (pid == 0)
            {
                execute_external_commands(cmd);
            }
            else if (pid > 0)
            {
                waitpid(pid, &status, WUNTRACED);

                if (WIFSTOPPED(status))
                {
                    stopped_jobs[stopped_count].pid = pid;
                    strcpy(stopped_jobs[stopped_count].cmd, cmd);

                    stopped_count++;

                    printf("\n[%d]+ Stopped %s\n", stopped_count, cmd);
                }
            }
        }
        else if (strncmp("ps1=", cmd, 4) == 0)
        {
            if ((cmd[4] == ' ') || (cmd[4] == 0) || (cmd[4] == '_'))
            {
                printf(RED "Invalid command !\n");
                RESET;

                continue;
            }
            char temp_cmd[20];
            int i = 4, j = 0, flag = 0;
            while (cmd[i])
            {
                if (cmd[i] == ' ')
                {
                    flag = 1;
                    break;
                }
                temp_cmd[j] = cmd[i];
                i++;
                j++;
            }
            if (flag == 1)
            {
                printf(RED "Invalid command !\n");
                RESET;
                continue;
            }

            temp_cmd[j] = '\0';
            strcpy(prompt, temp_cmd);
        }
        else if (strcmp(cmd, "jobs") == 0)
        {
            jobs();
            continue;
        }
        else
        {
            printf(RED "Invalid command !\n");
            RESET;
            continue;
        }
    }
}

void execute_internal_commands(char *input_string)
{
    if (strcmp(input_string, "cd") == 0)
    {
        chdir(input_string);
    }
    if (strcmp(input_string, "pwd") == 0)
    {
        getcwd(input_string, 100);
    }
    if (strcmp(input_string, "exit") == 0)
    {
        exit(0);
    }
    if (strcmp(input_string, "echo $shell") == 0)
    {
        printf("%s\n", getenv("SHELL"));
    }
    if (strcmp(input_string, "echo $$") == 0)
    {
        printf("%d\n", getpid());
    }
    if (strcmp(input_string, "echo $?") == 0)
    {
        printf("%d\n", WEXITSTATUS(status));
    }
}
void signal_handler(int sig)
{
    if (sig == SIGINT)
    {
        if (pid > 0)
        {
            kill(pid, SIGINT);
        }
        else
        {
            printf("\nminishell$ : ");
            fflush(stdout);
        }
    }

    else if (sig == SIGTSTP)
    {
        if (pid > 0)
        {
            printf("\n");
        }
    }

    else if (sig == SIGCHLD)
    {
        int child;
        while ((child = waitpid(-1, &status, WNOHANG)) > 0)
        {
            if (WIFEXITED(status))
            {
                printf("Child %d exited\n", child);
            }
        }
    }
}

void execute_external_commands(char *input)
{
    /* check if pipe exists */
    if (strchr(input, '|') == NULL)
    {
        char *args[20];
        int i = 0;

        char *token = strtok(input, " ");
        while (token != NULL)
        {
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        args[i] = NULL;

        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }

    char *commands[10];
    int cmd_count = 0;

    char *token = strtok(input, "|");

    while (token != NULL)
    {
        commands[cmd_count++] = token;
        token = strtok(NULL, "|");
    }

    int fd[2];
    int prev_fd = -1;

    for (int i = 0; i < cmd_count; i++)
    {
        pipe(fd);

        pid_t child = fork();

        if (child == 0)
        {
            /* child process */

            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (i != 0)
            {
                dup2(prev_fd, 0);
                close(prev_fd);
            }

            if (i != cmd_count - 1)
            {
                dup2(fd[1], 1);
            }

            close(fd[0]);

            char *args[20];
            int j = 0;

            char *t = strtok(commands[i], " ");

            while (t != NULL)
            {
                args[j++] = t;
                t = strtok(NULL, " ");
            }

            args[j] = NULL;

            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }

        else
        {
            /* parent */

            waitpid(child, &status, WUNTRACED);

            if (WIFSTOPPED(status))
            {
                stopped_jobs[stopped_count].pid = child;
                strcpy(stopped_jobs[stopped_count].cmd, cmd);

                stopped_count++;

                printf("\n[%d]+ Stopped %s\n", stopped_count, cmd);
            }

            close(fd[1]);

            if (prev_fd != -1)
                close(prev_fd);

            prev_fd = fd[0];
        }
    }

    exit(0);
}
void jobs()
{
    if (stopped_count == 0)
    {
        printf("No stopped jobs\n");
        return;
    }

    for (int i = 0; i < stopped_count; i++)
    {
        printf("[%d] Stopped %s [%d]\n",
               i + 1,
               stopped_jobs[i].cmd,
               stopped_jobs[i].pid);
    }

    int job_no;
    printf("Select job number: ");
    scanf("%d", &job_no);

    if (job_no < 1 || job_no > stopped_count)
    {
        printf("Invalid job\n");
        return;
    }

    int option;

    printf("1. fg\n2. bg\nEnter option: ");
    scanf("%d", &option);

    struct job j = stopped_jobs[job_no - 1];

    if (option == 1) // foreground
    {
        kill(j.pid, SIGCONT);

        waitpid(j.pid, &status, WUNTRACED);

        printf("%s running in foreground\n", j.cmd);
    }
    else if (option == 2) // background
    {
        kill(j.pid, SIGCONT);

        printf("%s running in background\n", j.cmd);
    }

    for (int i = job_no - 1; i < stopped_count - 1; i++)
    {
        stopped_jobs[i] = stopped_jobs[i + 1];
    }

    stopped_count--;
}