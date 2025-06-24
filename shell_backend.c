
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXCOM 1000
#define MAXLIST 100

#define clear() printf("\033[H\033[J")

int interactive_mode = 1;

// Function declarations
int backend_main();
int passwordCheck();
void run_frontend_mode();
int processString(char *str, char **parsed, char **parsedpipe);
void execArgs(char **parsed);
void execArgsPiped(char **parsed, char **parsedpipe);
int ownCmdHandler(char **parsed);
int parsePipe(char *str, char **strpiped);
void parseSpace(char *str, char **parsed);
void openHelp();
void init_shell();
int takeInput(char *str);
void printDir();
char *getPasswordFromEnvFile();

// Shell logic
void init_shell()
{
    if (!interactive_mode)
        return;

    clear();
    printf("\n\n\n\n******************"
           "************************");
    printf("\n\n\n\t****MY SHELL****");
    printf("\n\n\t-USE AT YOUR OWN RISK-");
    printf("\n\n\n\n*******************"
           "***********************");
    char *username = getenv("USER");
    printf("\n\n\nUSER is: @%s", username);
    printf("\n");
    sleep(3);
    clear();
}

int takeInput(char *str)
{
    if (!interactive_mode)
        return 1;

    char *buf = readline("\n>>> ");
    if (buf && strlen(buf) != 0)
    {
        add_history(buf);
        strcpy(str, buf);
        free(buf);
        return 0;
    }
    return 1;
}

void printDir()
{
    if (!interactive_mode)
        return;

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s", cwd);
}

void execArgs(char **parsed)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        printf("\nFailed forking child..");
        return;
    }
    else if (pid == 0)
    {
        if (execvp(parsed[0], parsed) < 0)
        {
            printf("\nCould not execute command..");
        }
        exit(0);
    }
    else
    {
        wait(NULL);
    }
}

void execArgsPiped(char **parsed, char **parsedpipe)
{
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0)
    {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0)
    {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0)
        {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    }
    else
    {
        p2 = fork();

        if (p2 < 0)
        {
            printf("\nCould not fork");
            return;
        }

        if (p2 == 0)
        {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0)
            {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        }
        else
        {
            wait(NULL);
            wait(NULL);
        }
    }
}

void openHelp()
{
    printf("\n***WELCOME TO MY SHELL HELP***"
           "\n-Use the shell at your own risk..."
           "\nList of Commands supported:"
           "\n>cd"
           "\n>ls"
           "\n>exit"
           "\n>all other general commands available in UNIX shell"
           "\n>pipe handling"
           "\n>improper space handling");
}

int ownCmdHandler(char **parsed)
{
    int NoOfOwnCmds = 4, i, switchOwnArg = 0;
    char *ListOfOwnCmds[NoOfOwnCmds];
    char *username;

    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "hello";

    for (i = 0; i < NoOfOwnCmds; i++)
    {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0)
        {
            switchOwnArg = i + 1;
            break;
        }
    }

    switch (switchOwnArg)
    {
    case 1:
        printf("\nGoodbye\n");
        exit(0);
    case 2:
        chdir(parsed[1]);
        return 1;
    case 3:
        openHelp();
        return 1;
    case 4:
        username = getenv("USER");
        printf("\nHello %s.\nMind that this is "
               "not a place to play around.\nUse help to know more..\n",
               username);
        return 1;
    default:
        break;
    }
    return 0;
}

int parsePipe(char *str, char **strpiped)
{
    for (int i = 0; i < 2; i++)
    {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    return (strpiped[1] == NULL) ? 0 : 1;
}

void parseSpace(char *str, char **parsed)
{
    for (int i = 0; i < MAXLIST; i++)
    {
        parsed[i] = strsep(&str, " ");
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

int processString(char *str, char **parsed, char **parsedpipe)
{
    char *strpiped[2];
    int piped = parsePipe(str, strpiped);

    if (piped)
    {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);
    }
    else
    {
        parseSpace(str, parsed);
    }

    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}

char *getPasswordFromEnvFile()
{
    FILE *file = fopen(".env", "r");
    if (!file)
    {
        printf("Could not open .env file.\n");
        exit(1);
    }

    static char password[100];
    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "SHELL_PASSWORD=", 15) == 0)
        {
            strcpy(password, line + 15);
            password[strcspn(password, "\n")] = 0;
            fclose(file);
            return password;
        }
    }

    fclose(file);
    printf("SHELL_PASSWORD not found in .env file.\n");
    exit(1);
}

int passwordCheck()
{
    char input[100];
    char *real_password = getPasswordFromEnvFile();

    printf("Enter password to access shell: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    if (strcmp(input, real_password) == 0)
    {
        return 1;
    }
    return 0;
}

// Entry point for frontend execution mode
void run_frontend_mode()
{
    if (!passwordCheck())
    {
        fprintf(stderr, "AUTH_FAILED\n");
        exit(1);
    }

    char input[MAXCOM];
    while (fgets(input, sizeof(input), stdin))
    {
        input[strcspn(input, "\n")] = 0;

        char *parsedArgs[MAXLIST] = {0};
        char *parsedArgsPiped[MAXLIST] = {0};
        int execFlag = processString(input, parsedArgs, parsedArgsPiped);

        if (execFlag == 1)
            execArgs(parsedArgs);
        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);

        printf("CMD_COMPLETE\n");
        fflush(stdout);
    }
}

int backend_main()
{
    interactive_mode = 1;
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    char *parsedArgsPiped[MAXLIST];
    int execFlag = 0;

    if (!passwordCheck())
    {
        printf("Incorrect password. Access denied.\n");
        return 0;
    }

    init_shell();

    while (1)
    {
        printDir();
        if (takeInput(inputString))
            continue;
        execFlag = processString(inputString, parsedArgs, parsedArgsPiped);
        if (execFlag == 1)
            execArgs(parsedArgs);
        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);
    }
}

int main(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "--frontend") == 0)
    {
        interactive_mode = 0;
        run_frontend_mode();
        return 0;
    }
    else
    {
        return backend_main();
    }
}
