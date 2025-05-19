#include <linux/uaccess.h>

#include "binder_ioctl.h"

static int stored_val;

long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
  int val;
  switch (cmd) {
  case CMD_SET_VAL:
    if (copy_from_user(&val, (int __user *)arg, sizeof(val)))
      return -EFAULT;
    stored_val = val;
    pr_info("Value set to %d\n", stored_val);
    break;
  case CMD_GET_VAL:
    val = stored_val;
    if (copy_to_user((int __user *)arg, &val, sizeof(val)))
      return -EFAULT;
    pr_info("Value %d returned\n", val);
    break;
  case CMD_DO_ACTION:
    pr_info("Action performed with value %d\n", stored_val);
    break;
  default:
    return -ENOTTY;
  }
  return 0;
}