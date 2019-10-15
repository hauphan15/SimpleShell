#include  <stdio.h>
#include  <sys/types.h>
#include  <fcntl.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <string.h>

void  parse(char *line, char **argv);
void  execute(char **argv);

void  copyArr(char** des, char** source);
void  checkDoubleExclmt(char** argv, char** history);

void  redirectOut(char** argv);
void  redirectIn(char** argv);
void  cutCmd(char** argv);
int   checkCmd(char** argv);
char* getFileName(char** argv);

void  getCmd1Pipe(char** argv, char** cmd1);
void  getCmd2Pipe(char** argv, char** cmd2);
void  runPipeCmd(char** cmd1, char**  cmd2, char** argv);
void  runsource(int pfd[], char** cmd1);
void  rundest(int pfd[], char** cmd2);


void  main(void)
{
     char  line[1024];             
     char  *argv[64];            
     char  *history[64];
    
     char  *cmd1[64];
     char  *cmd2[64];

     while (1) {                   
          printf("Shell -> ");    
          gets(line);              
          printf("\n");
          parse(line, argv);
   
          if (strcmp(argv[0], "exit") == 0) 
               exit(0);
          
          checkDoubleExclmt(argv, history);

          int k = checkCmd(argv);
          switch(k)
          {
             case 0:
                 execute(argv);
                 break;

             case 1:
                 redirectOut(argv);
                 break;

             case 2:
                 redirectIn(argv);
                 break;

             case 3:
                 runPipeCmd(cmd1, cmd2, argv);
                 break;
          }
     }
}


void  redirectOut(char** argv)
{
    int out = open(getFileName(argv), O_RDWR|O_CREAT|O_APPEND, 0600);
    if (-1 == out) { perror("opening"); return 255; }

    int save_out = dup(fileno(stdout));

    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }

    cutCmd(argv);
    execute(argv);

    fflush(stdout); 
    close(out);
   
    dup2(save_out, fileno(stdout));
   
    close(save_out);
}

void  redirectIn(char** argv)
{
   argv[1] = NULL;
   argv[1] = strdup(argv[2]);
   argv[2] = NULL;
   execute(argv);
}


void  parse(char *line, char **argv)
{
     while (*line != '\0') {       
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     
          *argv++ = line;        
          while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') 
               line++;            
     }
     *argv = '\0';                 
}


void  execute(char **argv)
{
     pid_t  pid;
     int    status;

     if ((pid = fork()) < 0) {
          printf("*** ERROR: forking child process failed\n");
          exit(1);
     }
     else if (pid == 0) {                
          if (execvp(*argv, argv) < 0) {    
               printf("*** ERROR: exec failed\n");
               exit(1);
          }
     }
     else {                                
          while (wait(&status) != pid)     
               ;
     }
}

int checkCmd(char** argv)
{
   int i = 0;
   while(i < 64)
   {
      if(argv[i] == NULL)
      {
          return 0;
      }
      else if (strcmp(argv[i], ">") == 0)
      {
          return 1;
      }
      else if (strcmp(argv[i], "<") == 0)
      {
          return 2;
      }
      else if (strcmp(argv[i], "|") == 0)
      {
          return 3;
      }
      i++;
   }
}

char* getFileName(char** argv)
{
   int i = 0;
   while(i < 64)
   {
      if(argv[i] == NULL)
      {
          i = i - 1;
          return argv[i];
      }
      i++;
   }
}


void cutCmd(char** argv)
{
   int i = 0;
   while(i < 64)
   {
      if (strcmp(argv[i], ">") == 0)
      {
         int j = i;
         while(j < 64)
         {
           argv[j] = NULL;
           j++;
         }
         break;
      }
      i++;
   }
}


void copyArr(char** des, char** source)
{
   int i = 0;
   while(i < 64)
   {
      if(source[i] == NULL)
      {
         des[i] = NULL;
         break;
      }
      des[i] = strdup(source[i]);
      i++;
   }
}



void checkDoubleExclmt(char** argv, char** history)
{
    if(strcmp(argv[0], "!!") == 0)
    {
        if(history[0] == NULL)
        {
           printf("\nNo Command in history\n");
        }
        else
        {
           copyArr(argv, history);
        }
    }
    else
    {   
        copyArr(history, argv);
    }

}

///////////////pipe

void getCmd1Pipe(char** argv, char** cmd1)
{
    int i = 0;
    while(i < 64)
    { 
        if(strcmp(argv[i], "|") == 0)
        {   
            cmd1[i] = NULL;
            break;
        }

        cmd1[i] = strdup(argv[i]);
        i++;
    }
}

void getCmd2Pipe(char** argv, char** cmd2)
{
    int i = 0;

    while(i < 64)
    {  
       if(strcmp(argv[i], "|") == 0)
       {
           int j = i + 1;
           int k = 0;
           while(j < 64)
           {
                if(argv[j] == NULL)
                {   
                    cmd2[k] = NULL;
 	            break;    
 		}
                 
                cmd2[k] = strdup(argv[j]);
                k++;
                j++;
           }

           break;
       }
       i++;
    }
}


void runPipeCmd(char** cmd1, char** cmd2, char** argv)
{
    int pid, status;
    int fd[2]; 

    pipe(fd);

    getCmd1Pipe(argv, cmd1);
    getCmd2Pipe(argv, cmd2);

    runsource(fd, cmd1);
    rundest(fd, cmd2); 

    close(fd[0]); 
    close(fd[1]); 

    while ((pid = wait(&status)) != -1){}
}


void runsource(int pfd[], char** cmd1)
{ 
   int pid; 
   switch (pid = fork())
   { 
       case 0: 
        dup2(pfd[1], 1); 
        close(pfd[0]); 
        execvp(cmd1[0], cmd1); 
        perror(cmd1[0]); 

      default:
        break; 

      case -1: 
        perror("fork"); 
        exit(1); 
   } 
} 


void rundest(int pfd[], char** cmd2) 
{ 
    int pid; 
    switch (pid = fork())
    { 
       case 0: 
          dup2(pfd[0], 0); 
          close(pfd[1]); 
          execvp(cmd2[0], cmd2); 
          perror(cmd2[0]);

      default: 
          break; 

      case -1: 
          perror("fork"); 
          exit(1); 
    } 
}
