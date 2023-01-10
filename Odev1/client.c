#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>

#define MAX 10
#define TRUE 1
#define FALSE 0

struct message
{
  long mtype;
  char msgArray[1000];
};

int main()
{
  struct message sendingMessage;
  struct message recievingMessage;
  struct message *recievingMessageptr;

  void *bufptr;
  int buflen;

  key_t key;
  int msgid;
  int n;
  

  
  buflen = sizeof(struct message) - sizeof(long);
  bufptr = (void *) malloc(buflen);

  key = ftok("client.c", ',');
  msgid = msgget(key, 0666 | IPC_CREAT);

  if (msgid == -1)
  {
    perror("msgget");
    return EXIT_FAILURE;
  }

  while (TRUE)
  {
    
    start = time(NULL);
    terminate = 1;
    while (terminate) 
    {
      end = time(NULL);
      elapsed = difftime(end, start);
      if (elapsed >= 10.0 ) // seconds 
        terminate = 0;
      else 
        usleep(50000);
    }
    
    sendingMessage.mtype = 1;
    srand(time(NULL));
    for (int i = 0 ; i < 1000 ; i++) {
      
      char c = (char)((rand() % 256)-128);
      if(c == '\0'){
        i--;
        continue;
      } 
      sendingMessage.msgArray[i] = c;
    }
    
    n = msgsnd(msgid, (void *) &sendingMessage, buflen, 0);
    
    printf("Unsorted Array: ");
    for (int i = 0; i < 1000; i++) {
      printf("%d ", (int) sendingMessage.msgArray[i]);
    }
    printf("\n");

    if (n == -1)
    {
      perror("msgsnd");
      return EXIT_FAILURE;
    }

    n = msgrcv(msgid, bufptr, buflen, 1, 0);
    
    if(n == -1) 
    {
      perror("msgrcv");
      return EXIT_FAILURE;
    }

    recievingMessageptr = (struct message *) bufptr;
    recievingMessage = *recievingMessageptr;
      printf("Sorted Array: ");
    for (int i = 0; i < 1000; i++) {
      printf("%d ", (int) recievingMessage.msgArray[i]);
    }
    printf("\n\n\n");

  }
  free(bufptr);
  return 0; 
  
}