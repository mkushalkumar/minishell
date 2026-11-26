#include "main.h"

void extract_external_commands(char **external_commands)
{
    int fd = open("ext.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return;
    }

    char ch;
    char *buffer = NULL;
    int i = 0;
    int cmd_index = 0;

    while (read(fd, &ch, 1) > 0)
    {
        if (ch == '\r')
            continue;

        buffer = realloc(buffer, i + 2);
        if (buffer == NULL)
        {
            perror("realloc");
            close(fd);
            return;
        }

        if (ch == '\n')
        {
            buffer[i] = '\0';
            external_commands[cmd_index++] = buffer;

            buffer = NULL;
            i = 0;
        }
        else
        {
            buffer[i++] = ch;
        }
    }

    if (i > 0)
    {
        buffer[i] = '\0';
        external_commands[cmd_index++] = buffer;
    }

    external_commands[cmd_index] = NULL;

    close(fd);
}

int check_command_type(char *command, char **external_commands)
{

    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval", "set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source", "exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};
    for (int i = 0; external_commands[i] != NULL; i++)
    {
        if (strcmp(command, external_commands[i]) == 0)
        {

            return 1;
        }
    }

    for (int i = 0; builtins[i] != NULL; i++)
    {
        if (strcmp(command, builtins[i]) == 0)
        {
            return 2;
        }
    }
}




