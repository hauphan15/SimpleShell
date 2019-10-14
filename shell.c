#include  <stdio.h>
#include  <sys/types.h>
#include  <fcntl.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <string.h>

void  parse(char *line, char **argv);
void  execute(char **argv);
void  print(char** arr);
void  copyArr(char** des, char** source);
void  checkDoubleExclmt(char** argv, char** history);
void  redirectOut(char** argv);
void  redirectIn(char** argv);
void  freeArr(char** arr);
void  cutCmd(char** argv);
int   checkRedCmd(char** argv);
char* getFileName(char** argv);


void  main(void)
{
     char  line[1024];             
     char  *argv[64];            
     char  *history[64];

     while (1) {                   
          printf("Shell -> ");    
          gets(line);              
          printf("\n");
          parse(line, argv);
   
          if (strcmp(argv[0], "exit") == 0) 
               exit(0);
          
          printf("1----%s\n", argv[0]);
          printf("1----%s\n", history[0]);
          checkDoubleExclmt(argv, history);
          printf("2----%s\n", argv[0]);
          printf("2----%s\n", history[0]);

          int k = checkRedCmd(argv);
          if (k == 1) 
          {  
             printf("redirectOut\n"); 
             redirectOut(argv);
          }
          else if (k == 2) 
          {  
             printf("redirectIN\n"); 
             redirectIn(argv);
          }
          else
          {
             printf("execute\n");
             execute(argv);
          }

         
     }
}


void  redirectOut(char** argv)
{
    int out = open(getFileName(argv), O_RDWR|O_CREAT|O_APPEND, 0600);
    if (-1 == out) { perror("opening cout.log"); return 255; }

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
   argv[1] = argv[2];
   argv[2] = NULL;
   execute(argv);
}


void  parse(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */ 
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;          /* save the argument position     */
          while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n') 
               line++;             /* skip the argument until ...    */
     }
     *argv = '\0';                 /* mark the end of argument list  */
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


void print(char** arr)
{
  int i = 0;
  for(i; i < sizeof(arr)/4; i++)
    printf("\n %s %d", arr[i], i);
}


int checkRedCmd(char** argv)
{
   int i = 0;
   while(i < 64)
   {
      if (strcmp(argv[i], ">") == 0)
      {
          return 1;
      }
      if (strcmp(argv[i], "<") == 0)
      {
          return 2;
      }
      i++;
   }
   return 0;
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
      des[i] = source[i];
      i++;
   }
}

void freeArr(char** arr)
{
    int i = 0;
    for(i; i < 64; i++)
       free(arr[i]);
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










