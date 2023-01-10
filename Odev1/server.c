#include <stdio.h>
#include <stdlib.h>  
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <string.h>
#include <pthread.h>

#define MAX 10
#define TRUE 1
#define FALSE 0

struct message {
    long mtype;
    char msgArray[1000];
};

struct message sendingMessage;
struct message recievingMessage;
struct message *recievingMessageptr;

void *bufptr;
int buflen;

key_t key;
int msgid;
int n;

FILE *out_file;

void* func(void* arg)
{
  for (int i = 0; i < 1000; i++) 
  {
    sendingMessage.msgArray[i] = recievingMessage.msgArray[i];
  }  
  
  for (int i = 0; i < 1000; i++) 
  {
    for (int j = i + 1; j < n; j++)
    {
      if (sendingMessage.msgArray[i] > sendingMessage.msgArray[j]) 
      {
        char a =  sendingMessage.msgArray[i];
        sendingMessage.msgArray[i] = sendingMessage.msgArray[j];
        sendingMessage.msgArray[j] = a;

      }
    }
  }
  
  out_file = fopen("PID.txt", "a");
  if (out_file == NULL) 
  {   
    printf("Error! Could not open file\n"); 
    exit(-1);
  }
  fprintf(out_file, "Sorted array: ");
  
  for(int i = 0 ; i < 1000 ; i++) {
    fprintf(out_file, "%d ",(int) sendingMessage.msgArray[i]); 
  }
  
  fprintf(out_file, "\n");
  fclose(out_file); 
  
  sendingMessage.mtype = 1;

  n = msgsnd(msgid,(void *) &sendingMessage,buflen, 0);
  
  if (n == -1)
  {
    perror("msgget");
    exit(-1);
  }
  
  pthread_exit(NULL);
}

int main() {
  
  pthread_t ptid;

  buflen = sizeof(struct message) - sizeof(long);
  bufptr = (void *) malloc(buflen);

  key = ftok("client.c", ',');
  msgid = msgget(key, 0666 | IPC_CREAT);

  while(TRUE) {
    n = msgrcv(msgid, bufptr, buflen, 1, 0);

    if(n == -1) 
    {
      perror("msgrcv");
      return EXIT_FAILURE;
    }
    recievingMessageptr = (struct message *) bufptr;
    recievingMessage = *recievingMessageptr;

        
    pthread_create(&ptid, NULL, &func, NULL);
    pthread_join(ptid, NULL);

  }
  free(bufptr);
  return 0; 
}  