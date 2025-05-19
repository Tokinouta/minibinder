#include "minibinder.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int binder_send(pid_t target_pid, const void *data, size_t data_len) {
  int fd = open("/dev/minibinder", O_WRONLY);
  if (fd < 0)
    return -1;

  pid_t current_pid = getpid();

  const size_t data_size =
      sizeof(target_pid) + sizeof(current_pid) + sizeof(data_len) + data_len;
  char buf[data_size];
  memcpy(buf, &target_pid, sizeof(target_pid));
  memcpy(buf + sizeof(target_pid), &current_pid, sizeof(current_pid));
  memcpy(buf + sizeof(target_pid) + sizeof(current_pid), &data_len,
         sizeof(data_len));
  memcpy(buf + sizeof(target_pid) + sizeof(current_pid) + sizeof(data_len),
         data, data_len);

  ssize_t ret = write(fd, buf, sizeof(buf));
  close(fd);
  return ret == (ssize_t)sizeof(buf) ? 0 : -1;
}

int binder_receive(pid_t *sender_pid, void *data, size_t *data_len) {
  int fd = open("/dev/minibinder", O_RDONLY);
  if (fd < 0)
    return -1;

  const size_t data_size =
      sizeof(pid_t) + sizeof(pid_t) + sizeof(size_t) + *data_len;

  char buf[data_size];
  ssize_t n = read(fd, buf, sizeof(buf));
  printf("Read %zd bytes from minibinder\n", n);
  close(fd);

  if (n < (ssize_t)(sizeof(pid_t) + sizeof(pid_t) + sizeof(size_t)))
    return -1;

  *sender_pid = *(pid_t *)buf;
  *data_len = *(size_t *)(buf + sizeof(pid_t) + sizeof(pid_t));

  printf("Received data length: %zu\n", *data_len);

  if (*data_len > MAX_DATA_SIZE)
    return -1;
  // if (n != (ssize_t)data_size)
  //   return -1;

  memcpy(data, buf + sizeof(pid_t) + sizeof(pid_t) + sizeof(size_t), *data_len);
  return 0;
}