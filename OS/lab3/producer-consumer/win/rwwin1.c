#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <stdbool.h>
#include <Winuser.h>

volatile bool flag = true;

HANDLE canReadEvent;
HANDLE canWriteEvent;
HANDLE readers_threads[5];
HANDLE writers_threads[3];
int readers_id[5];
int writers_id[3];
LONG active_reader = 0;
LONG activeWriter = FALSE;
HANDLE mutex;

char word = 'a' - 1;

void startRead()
{
  if (activeWriter || WaitForSingleObject(canWriteEvent, 0))
  {
    WaitForSingleObject(canReadEvent, INFINITE);
  }

  active_reader++;

  SetEvent(canReadEvent);

  WaitForSingleObject(mutex, INFINITE);
}

void stopRead()
{
  ReleaseMutex(mutex);

  active_reader--;

  if (active_reader == 0)
  {
    SetEvent(canWriteEvent);
  }
}

void startWrite()
{
  if (active_reader > 0 || activeWriter)
  {
    WaitForSingleObject(canWriteEvent, INFINITE);
  }

  activeWriter = TRUE;

  WaitForSingleObject(mutex, INFINITE);
}

void stopWrite()
{
  ReleaseMutex(mutex);

  activeWriter = FALSE;

  if (WaitForSingleObject(canReadEvent, 0))
  {
    SetEvent(canReadEvent);
  }
  else
  {
    SetEvent(canWriteEvent);
  }
}

DWORD Reader(PVOID param)
{
  int id = *(int*)param;
  while (flag)
  {
    startRead();
    printf("%d read %c\n", id, word);
    stopRead();
  }
  printf("Reader %d stopped\n", id);
  return 0;
}

DWORD Writer(PVOID param)
{
  int id = *(int*)param;
  while (flag)
  {
    startWrite();
    if (word == 'z')
      word = 'a';
    else
      word++;
    printf("%d wrote %c\n", id,  word);
    stopWrite();
  }
  printf("Writer %d stopped\n", id);
  return 0;
}

VOID CALLBACK AlarmHandler(PVOID lpParam, BOOLEAN TimerOrWaitFired) {
  printf("\nAlarm.\n");
  flag = FALSE;
}

int main(void)
{
  setbuf(stdout, NULL);

  if ((canReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
  {
    printf("CreateEvent");
    exit(1);
  }
  if ((canWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
  {
    printf("CreateEvent");
    exit(1);
  }
  if ((mutex = CreateMutex(NULL, 0, NULL)) == NULL)
  {
    printf("CreateMutex");
    exit(1);
  }

  for (int i = 0; i < 3; i++)
  {
    writers_id[i] = i;
    writers_threads[i] = CreateThread(NULL, 0, Writer, &writers_id[i], 0, NULL);

  }

  for (int i = 0; i < 5; i++)
  {
    readers_id[i] = i;
    readers_threads[i] = CreateThread(NULL, 0, Reader, &readers_id[i], 0, NULL);

  }

  HANDLE timerQueue = CreateTimerQueue();
  HANDLE timer = NULL;

  CreateTimerQueueTimer(&timer, timerQueue, AlarmHandler, NULL, 1000, 0, 0);

  WaitForMultipleObjects(5, readers_threads, TRUE, INFINITE);
  WaitForMultipleObjects(2, writers_threads, TRUE, INFINITE);


  CloseHandle(canReadEvent);
  CloseHandle(canWriteEvent);
  CloseHandle(mutex);

  ExitProcess(0);
}
