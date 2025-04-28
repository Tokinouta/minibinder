#include "linux/gfp_types.h"
#include "linux/list.h"
#include "linux/uaccess.h"
#include <linux/fs.h>         // Include the header for struct file_operations
#include <linux/miscdevice.h> // Include the header for misc_register and miscdevice
#include <linux/mutex.h>
#include <linux/mutex_types.h>
#include <linux/types.h>

#define MAX_DATA_SIZE 1024

struct message {
  pid_t sender_pid;
  pid_t target_pid;
  char data[MAX_DATA_SIZE];
  size_t data_size;
  struct list_head list;
};

struct target_pid_entry {
  pid_t pid;
  struct list_head message_list;
  struct mutex lock;
  wait_queue_head_t waitq;
  struct list_head list;
};

static LIST_HEAD(target_pid_list);
static DEFINE_MUTEX(target_pid_list_lock);

/**
 * get_target_pid_entry - Find or create a target_pid_entry for a given PID.
 * @pid:    The process ID to look up or create an entry for.
 * @create: If true, create a new entry if one does not exist.
 *
 * This function searches the global target_pid_list for an entry matching the
 * specified PID. If found, it returns a pointer to the entry. If not found and
 * 'create' is true, it allocates and initializes a new target_pid_entry,
 * adds it to the global list, and returns a pointer to it. If allocation fails
 * or 'create' is false, returns NULL.
 *
 * The function uses target_pid_list_lock to ensure thread-safe access to the
 * global list.
 *
 * Return: Pointer to the found or newly created target_pid_entry, or NULL on
 * failure.
 */
static struct target_pid_entry *get_target_pid_entry(pid_t pid, bool create) {
  struct target_pid_entry *entry = NULL;
  mutex_lock(&target_pid_list_lock);

  list_for_each_entry(entry, &target_pid_list, list) {
    if (entry->pid == pid) {
      mutex_unlock(&target_pid_list_lock);
      pr_info("Found existing target_pid_entry for PID %d\n", pid);
      return entry;
    }
  }

  if (!create) {
    mutex_unlock(&target_pid_list_lock);
    pr_info(
        "No existing target_pid_entry found for PID %d and create is false\n",
        pid);
    return NULL;
  }

  entry = kmalloc(sizeof(*entry), GFP_KERNEL);
  if (!entry) {
    mutex_unlock(&target_pid_list_lock);
    pr_err("Failed to allocate memory for target_pid_entry\n");
    return NULL;
  }

  entry->pid = pid;
  INIT_LIST_HEAD(&entry->message_list);
  mutex_init(&entry->lock);
  init_waitqueue_head(&entry->waitq);
  list_add(&entry->list, &target_pid_list);
  mutex_unlock(&target_pid_list_lock);
  pr_info("Created new target_pid_entry for PID %d\n", pid);
  return entry;
}

static int minibinder_open(struct inode *inode, struct file *file) {
  pr_info("minibinder device opened\n");
  return 0;
}

static int minibinder_release(struct inode *inode, struct file *file) {
  pr_info("minibinder device closed\n");
  return 0;
}

/**
 * minibinder_read - Read a message for the current process from the minibinder
 * device.
 * @file:  Pointer to the file structure.
 * @buf:   User-space buffer to copy the message into.
 * @count: Size of the user-space buffer.
 * @ppos:  File position pointer (unused).
 *
 * This function retrieves the next available message for the calling process
 * (by PID) from its message queue. If no message is available:
 *   - In non-blocking mode, returns -EAGAIN.
 *   - In blocking mode, waits until a message arrives or the wait is
 * interrupted.
 *
 * The message format copied to user space is:
 *   [sender_pid][data_size][data]
 *
 * On success, the message is removed from the queue and its memory is freed.
 *
 * Return: Number of bytes read on success, negative error code on failure.
 */
static ssize_t minibinder_read(struct file *file, char __user *buf,
                               size_t count, loff_t *ppos) {
  pr_info("minibinder device read\n");
  struct target_pid_entry *entry;
  struct message *msg;
  size_t msg_size;
  int ret = 0;

  entry = get_target_pid_entry(current->pid, true);
  if (!entry) {
    pr_err("Failed to get target_pid_entry for current PID\n");
    return -ENOMEM;
  }

  mutex_lock(&entry->lock);
  while (list_empty(&entry->message_list)) {
    mutex_unlock(&entry->lock);
    if (file->f_flags & O_NONBLOCK) {
      pr_info("No messages available, non-blocking read\n");
      return -EAGAIN;
    }
    pr_info("Waiting for messages...\n");
    if (wait_event_interruptible(entry->waitq,
                                 !list_empty(&entry->message_list))) {
      pr_err("Wait event interrupted\n");
      return -ERESTARTSYS;
    }
    mutex_lock(&entry->lock);
  }

  msg = list_first_entry(&entry->message_list, struct message, list);
  msg_size = msg->data_size + sizeof(msg->sender_pid) + sizeof(msg->data_size);
  if (count < msg_size) {
    pr_err("Buffer too small for message\n");
    ret = -EINVAL;
    goto out;
  }

  if (copy_to_user(buf, &msg->sender_pid, sizeof(msg->sender_pid))) {
    pr_err("Failed to copy sender PID to user space\n");
    ret = -EFAULT;
    goto out;
  }
  if (copy_to_user(buf + sizeof(msg->sender_pid), &msg->data_size,
                   sizeof(msg->data_size))) {
    pr_err("Failed to copy data size to user space\n");
    ret = -EFAULT;
    goto out;
  }
  if (copy_to_user(buf + sizeof(msg->sender_pid) + sizeof(msg->data_size),
                   msg->data, msg->data_size)) {
    pr_err("Failed to copy message data to user space\n");
    ret = -EFAULT;
    goto out;
  }

  pr_info("Read message from PID %d, size %zu\n", msg->sender_pid,
          msg->data_size);
  list_del(&msg->list);
  kfree(msg);
  ret += msg_size;

out:
  mutex_unlock(&entry->lock);
  return 0;
}

static ssize_t minibinder_write(struct file *file, const char __user *buf,
                                size_t count, loff_t *ppos) {
  pr_info("minibinder device write\n");
  return count;
}

static struct file_operations binder_fops = {
    .owner = THIS_MODULE,
    .open = minibinder_open,
    .release = minibinder_release,
    .read = minibinder_read,
    .write = minibinder_write,
};

static struct miscdevice binder_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "minibinder",
    .fops = &binder_fops,
};

static int __init minibinder_init(void) {
  int ret;

  ret = misc_register(&binder_device);
  if (ret) {
    pr_err("Failed to register minibinder device\n");
    return ret;
  }

  pr_info("minibinder device registered with minor %d\n", binder_device.minor);
  return 0;
}

static void __exit minibinder_exit(void) {
  misc_deregister(&binder_device);
  pr_info("minibinder device unregistered\n");
}

module_init(minibinder_init);
module_exit(minibinder_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");