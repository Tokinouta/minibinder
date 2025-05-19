#ifndef BINDER_H
#define BINDER_H

#include <sys/types.h>

#define MAX_DATA_SIZE 512

int binder_send(pid_t target_pid, const void *data, size_t data_len);
int binder_receive(pid_t *sender_pid, void *data, size_t *data_len);
int set_value(int fd, int value);
int get_value(int fd, int *value);
int do_action(int fd);

#endif