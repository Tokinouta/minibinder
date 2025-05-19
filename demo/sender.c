#include "minibinder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <target_pid> <message>\n", argv[0]);
    return 1;
  }

  pid_t target_pid = atoi(argv[1]);
  const char *message = argv[2];
  size_t len = strlen(message) + 1;

  if (binder_send(target_pid, message, len) != 0) {
    perror("binder_send failed");
    return 1;
  }

  printf("Sent message to PID %d: %s\n", target_pid, message);
  return 0;
}