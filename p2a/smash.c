/**
 * @file smash.c
 * @author minghao {NetID:mtang64}  {mtang64@wisc.edu}
 * @brief  cs537 p2a
 * @version final 
 * @date 2020-02-14
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>



char *read_a_line(void);   // read a line from screen
char **parse_a_line(char *str);   // parse a line to different args and check if have ">"
void execute_program(char **argv, int redirect);  
int my_cd(char **argc);     //cd
int my_exit(void);          // call exit
void my_path(char **args);  // path choose
int exec_built_in_command(char **args); //build in (cd, exit, path)
//int check_built_in_command(char *str);
void execute_allocate(char *line); //judge if we have ";", "&"
void wait_for_kid_pids(void); //just wait for all kid-process to process
void path_add(char *add);
void path_remove(char *remove);
void path_clear(void);
void path_print(void);

char *PATH_ENV[256]; // all path are saved 
long kid_pids[64]; // save the pid of kid_process. go through all of them 
int kid_num; //the number of running kid_pids
int status;     // to exit the while 
char error_message[30] = "An error has occurred\n";

int main(int argc, char **argv)
{
    PATH_ENV[0]="/bin/";

    //rintf("%s\n", PATH_ENV);
     status =1;
    char *line;

    if (argc >1){
        status =0;
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            printf("cannot open file\n"); // if cannot open the file
            exit(1);
        }
        char *line = NULL;
        size_t linecap = 0;
        ssize_t linelen;

        while ((linelen = getline(&line, &linecap, fp)) >0)
        {
       //     printf("the line i get is :%s//\n", line);
         //   fflush(stdout);
            execute_allocate(line);
            wait_for_kid_pids();
        
        }

        if(line)
        free(line);
        fclose(fp);
    }
    else
    {
        while (status)
        {

            printf("smash> ");
            fflush(stdout);
            line = read_a_line();
            execute_allocate(line);
            wait_for_kid_pids();
            if (status == 0){
                exit(0);
                break;
            }
                

        } //while
    }


  
   
}

char *read_a_line(void){
        char *line = NULL;
        size_t linecap = 0;

        getline(&line, &linecap, stdin);
        return line;
    }

char **parse_a_line(char *str){
        // char *str = strdup("this is a   very long line");
        //  printf("str is at %p \n", str);
        char **arg_temp = malloc(256 * sizeof(char *));
        char *p=NULL; //= strsep(&str," ");
        int i = 0;

        for (p = strsep(&str, " > \n \t"); p != NULL; p = strsep(&str, " >\n \t"))
        {
            if (strlen(p) > 0)
            {
                //    printf("pares0:%s", p);
                arg_temp[i] = p;
                arg_temp[i+1] = NULL;
                //   printf("%s \n", p);
                //   printf("parse1:%s", arg_temp[i]);
                i++;
            }
        }
        free(str);
        return arg_temp;
}


void execute_allocate(char *line){

    char parallel[3] = "&";
    char multiple[3] = ";";
    char redirct[3]  = ">";
    char *command;
    char **args;
    int redirect =0;

    //this is for ";" to execute commands one by one
     if (strstr(line, multiple) != NULL)
    {
       // printf("multiple\n");
        for (command = strsep(&line, ";"); command != NULL; command = strsep(&line, ";"))
        {
            if (strlen(command) > 0)
            {
                //   printf("%s\n", command);
                //  args = parse_a_line(command);
                wait_for_kid_pids();

                execute_allocate(command);
                //   printf(";");
                //  printf("---------------\n");
            }
        }
    }
    else if (strstr(line, parallel) != NULL)
    {
       // printf("parllel\n");

        for (command = strsep(&line, "&"); command != NULL; command = strsep(&line, "&"))
        {
            if (strlen(command) > 0)
            {
             //   printf("%s\n", command);
                execute_allocate(command);
              
                //   args = parse_a_line(p);
            }
        }
    }
    else
    {
        if (strstr(line, redirct) != NULL)
        {
            redirect = 1;
        }
        else
        {
            redirect = 0;
        }

        //
        args = parse_a_line(line);

        if (args[0] != NULL){
            if (exec_built_in_command(args) == 0)
            {
                execute_program(args, redirect);
            }
        }

       
    }
}

void execute_program(char **argv, int redirect  ) //execute the binary file
{
      //  printf("starting from %d\n", getpid());
       // int stat = 1;
       int flag_exec = 0;
        int rc = fork();
       // int exec_rc;

        if (rc == 0)
        {
            //printf("%d\n", kid_num);
            if (redirect) //check if redirct
            {

                int i = 0;
                while (argv[i+1] != NULL)
                {
                    i++;
                }
            
                FILE *new_f = fopen(argv[i],"w");
                int newfd = fileno(new_f);
                argv[i] = NULL;

                dup2(newfd, 1);
                dup2(newfd, 2);
                // printf("smash> ");
                char *my_argv = malloc(256);
                if (PATH_ENV[0]==NULL){
                    exit(0);
                    write(STDERR_FILENO, error_message, strlen(error_message));
                }
                for (int i = 0; PATH_ENV[i] != NULL; i++)
                {
                    strcpy(my_argv, PATH_ENV[0]);
                    strcat(my_argv, "/");
                    strcat(my_argv, argv[0]);
                    if (access(my_argv, X_OK) == 0)
                    {
                       // printf("smash> ");
                        int exec_rc = execv(my_argv, argv);
                       // write(newfd, error_message, strlen(error_message));
                        //perror("execv error");

                        flag_exec =1;
                         if (exec_rc == -1)
                        {
                            write(newfd, error_message, strlen(error_message));
                        }
                    }
                    
                    }

                if(flag_exec == 0){
                    exit(0);
                  ///  write(STDERR_FILENO, error_message, strlen(error_message));
                    
                }
                close(newfd);
                        }
            else
            {
                        
                    char *my_argv = malloc(256);
                      if (PATH_ENV[0]==NULL){
                            
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            exit(0);
                      }
                
                    for (int i = 0; PATH_ENV[i] != NULL; i++)
                    {
                        strcpy(my_argv, PATH_ENV[0]);
                        strcat(my_argv, "/");
                        strcat(my_argv, argv[0]);
                        if (access(my_argv, X_OK) == 0)
                        {
                            
                            flag_exec = 1;

                            int exec_rc = execv(my_argv, argv);
                            if (exec_rc ==-1){
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            }
                            
                        }
                        
                    }

                    if(flag_exec == 0){
                        
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(0);
                    }
            }
                    //   if (exec_rc !=1 )
                    ///  printf("error");
        }     
        else
        {
            
            kid_pids[kid_num] = rc;
            kid_num++;

            //   int wait_rc = waitpid(rc, NULL, 0);
            //    printf("Exiting from my parent and my Pid is %d and RC is %d\n", getpid(), rc);
        }


}


int my_cd(char **argc)
{
        if (chdir(argc[1]) != 0)
        {
        write(STDERR_FILENO, error_message, strlen(error_message));
        }

    return 1;
}

int exec_built_in_command(char**args){
    if (strcmp(args[0], "exit") == 0){
        my_exit();
        return 1;
    }
    else if (strcmp(args[0], "path") == 0)
    {
        my_path(args);
        return 1;
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        my_cd(args);
        return 1;

    }
    else 
    return 0;
}

int my_exit(){
    status = 0;
    return 0;
}

void my_path(char **args){
            //add, remove, clear
if (strcmp(args[1], "add") == 0){

    path_add(args[2]);
   // printf("path add %s \n", PATH_ENV);
    }
else if (strcmp(args[1], "remove") == 0){
   // printf("path remove\n");
    path_remove(args[2]);
    }
else if (strcmp(args[1], "clear") == 0){
    path_clear();
        }
else{
    path_print();
    }
    
}

void wait_for_kid_pids(void){
    for (int j = 0; j < 64; j++)
    {
      //  int wait_rc = 
        waitpid(kid_pids[j], NULL, 0);
    }

    kid_num =0;
}

void path_add(char *add)
{
    int i = 0;
    int j = 0;
    for (i = 0; PATH_ENV[i] != NULL; i++)
        j++;

    for (i = j; i > 0; i--)
    {
        PATH_ENV[i] = PATH_ENV[i - 1];
    }

        PATH_ENV[0] = add;
}

void path_remove(char *remove)
{
    int i = 0, j = 0;
    for (i = 0; PATH_ENV[i] != NULL; i++)
    {
        //  printf("%s, %d\n", path_list[i], i);
        if (strcmp(PATH_ENV[i], remove) == 0)
            // printf( "%d \n", i);
            j = i;
    }

    for (i = j; PATH_ENV[i] != NULL; i++)
        PATH_ENV[i] = PATH_ENV[i + 1];
}

void path_clear(void)
{
    for (int i = 0; PATH_ENV[i] != NULL; i++)
        PATH_ENV[i] = NULL;
}

void path_print(void){
    for (int i = 0; PATH_ENV[i] != NULL; i++)
    {
          printf("%s\n", PATH_ENV[i]);
        }
}