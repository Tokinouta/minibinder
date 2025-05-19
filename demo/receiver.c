#include "minibinder.h"
#include <stdio.h>
#include <unistd.h>

int main() {
  printf("Receiver PID: %d\n", getpid());
  printf("Waiting for messages...\n");

  while (1) {
    pid_t sender_pid;
    char data[MAX_DATA_SIZE];
    size_t data_len =
        MAX_DATA_SIZE - (sizeof(pid_t) + sizeof(pid_t) + sizeof(size_t));

    if (binder_receive(&sender_pid, data, &data_len) == 0) {
      printf("Received from %d: %.*s\n", sender_pid, (int)data_len, data);
    } else {
      perror("Error receiving message");
      sleep(1);
    }
  }
  return 0;
}