#include "minibinder.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define MY_MAGIC 'k'
#define CMD_SET_VAL _IOW(MY_MAGIC, 1, int)
#define CMD_GET_VAL _IOR(MY_MAGIC, 2, int)
#define CMD_DO_ACTION _IO(MY_MAGIC, 3)

int main() {
  int fd = open("/dev/minibinder", O_WRONLY);
  if (fd < 0)
    return -1;

  int value = 42; // Example value to set
  set_value(fd, value);
  get_value(fd, &value);
  do_action(fd);
  return 0;
}