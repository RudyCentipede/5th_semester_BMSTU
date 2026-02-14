#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <Winuser.h>

volatile bool flag = true;

HANDLE canReadEvent;
HANDLE canWriteEvent;
HANDLE alpha_end;
HANDLE readers_threads[5];
HANDLE writers_threads[3];
int readers_id[5];
int writers_id[3];

LONG count_active_readers = 0;
LONG countWriters = 0;
LONG countReaders = 0;
LONG alpha_count = 0;

char word = 'a';

//BOOL WINAPI sig_handler(DWORD sig_num)
//{
//  flag = FALSE;
//  return TRUE;
//}

void startRead()
{
  InterlockedIncrement(&countReaders);

  if (countWriters > 0)
  {
    WaitForSingleObject(canReadEvent, INFINITE);
  }

  InterlockedDecrement(&countReaders);
  InterlockedIncrement(&count_active_readers);

  SetEvent(canReadEvent);
}

void stopRead()
{
  InterlockedDecrement(&count_active_readers);
  if (count_active_readers == 0)
    SetEvent(canWriteEvent);
}

void startWrite()
{
  InterlockedIncrement(&countWriters);
  if (count_active_readers > 0 || countWriters > 1)
  {
    WaitForSingleObject(canWriteEvent, INFINITE);
  }
}

void stopWrite()
{
  InterlockedDecrement(&countWriters);

  if (countWriters == 0)
  {
    if (countReaders > 0)
    {
      SetEvent(canReadEvent);
    }
    else
    {
      SetEvent(canWriteEvent);
    }
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
    {
      word = 'a';
      LONG current_alpha = InterlockedIncrement(&alpha_count);

      if (current_alpha > 2)
      {
        if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0))
        {
          perror("GenerateConsoleCtrlEvent");
          ExitThread(1);
        }
      }
    }
    else
    {
      word++;
    }

    printf("%d wrote %c\n", id, word);
    stopWrite();
  }
  printf("Writer %d stopped\n", id);
  return 0;
}

int main(void)
{
  setbuf(stdout, NULL);
  srand(GetTickCount());

  canReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
  canWriteEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
  alpha_end = CreateEvent(NULL, FALSE, FALSE, NULL);

  if (!canReadEvent || !canWriteEvent || !alpha_end)
  {
    printf("CreateEvent failed\n");
    exit(1);
  }



  for (int i = 0; i < 3; i++)
  {
    writers_id[i] = i;
    writers_threads[i] = CreateThread(NULL, 0, Writer, &writers_id[i], 0, NULL);
    if (writers_threads[i] == NULL)
    {
      printf("CreateThread for writer %d failed\n", i);
      exit(1);
    }
  }

  for (int i = 0; i < 5; i++)
  {
    readers_id[i] = i;
    readers_threads[i] = CreateThread(NULL, 0, Reader, &readers_id[i], 0, NULL);
    if (readers_threads[i] == NULL)
    {
      printf("CreateThread for reader %d failed\n", i);
      exit(1);
    }
  }


  DWORD result = WaitForSingleObject(alpha_end, INFINITE);

  if (result == WAIT_OBJECT_0)
  {
    flag = FALSE;
  }


  WaitForMultipleObjects(5, readers_threads, TRUE, INFINITE);
  WaitForMultipleObjects(3, writers_threads, TRUE, INFINITE);


  CloseHandle(canReadEvent);
  CloseHandle(canWriteEvent);
  CloseHandle(alpha_end);

  for (int i = 0; i < 3; i++)
    CloseHandle(writers_threads[i]);
  for (int i = 0; i < 5; i++)
    CloseHandle(readers_threads[i]);

  printf("Program terminated successfully.\n");
  return 0;
}