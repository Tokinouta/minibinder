#include <linux/fs.h>
#include <linux/ioctl.h>

#define MY_MAGIC 'k'
#define CMD_SET_VAL _IOW(MY_MAGIC, 1, int)
#define CMD_GET_VAL _IOR(MY_MAGIC, 2, int)
#define CMD_DO_ACTION _IO(MY_MAGIC, 3)

long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
