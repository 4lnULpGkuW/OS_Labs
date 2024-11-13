#include <linux/kernel.h>
#include <linux/module.h>      // Для всех модулей
#include <linux/printk.h>      // Для pr_info()
#include <linux/proc_fs.h>     // Для работы с procfs
#include <linux/uaccess.h>     // Для copy_to_user()
#include <linux/version.h>     // Для проверки версии ядра
#include <linux/types.h>       // Для ssize_t
#include <linux/time.h>        // Для работы с временем в ядре
#include <linux/slab.h>        // Для kmalloc

#define procfs_name "tsu"
static struct proc_dir_entry *our_proc_file = NULL;

MODULE_LICENSE("GPL");

bool parity = false;

static ssize_t procfile_read(
    struct file *file_pointer, char __user *buffer,
    size_t buffer_length, loff_t* offset)
{
    char *msg;
    size_t len;

    struct timespec64 ts;
    struct tm buf;
    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &buf);

    if (!parity)
    {
        len = snprintf(NULL, 0, "seconds: %d\n", buf.tm_sec);
        msg = kmalloc(len + 1, GFP_KERNEL);
        snprintf(msg, len + 1, "seconds: %d\n", buf.tm_sec);
    }
    else
    {
        len = snprintf(NULL, 0, "minutes: %d\n", buf.tm_min);
        msg = kmalloc(len + 1, GFP_KERNEL);
        snprintf(msg, len + 1, "minutes: %d\n", buf.tm_min);
    }

    if (*offset == 0) {
        parity = !parity;
    }

    if (*offset >= len) {
        kfree(msg);
        return 0;
    }

    size_t copy_len = min(len - *offset, buffer_length);

    if (copy_to_user(buffer, msg + *offset, copy_len)) {
        kfree(msg);
        return -EFAULT;
    }

    kfree(msg);
    pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name);
    *offset += copy_len;  
    return copy_len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static struct proc_ops proc_acpi_operations = {
        .proc_read = procfile_read,
};
#else
static struct file_operations proc_acpi_operations = {
        .read = procfile_read,
};
#endif

static int __init procfs1_init(void)
{
    pr_info("Welcome to the Tomsk State University\n");
    our_proc_file = proc_create(procfs_name, 0644, NULL, &proc_acpi_operations);
    if (our_proc_file == NULL) {
        pr_alert("Error: Could not initialize /proc/%s\n", procfs_name);
        return -ENOMEM;
    }
    pr_info("/proc/%s created\n", procfs_name);
    return 0;
}

static void __exit procfs1_exit(void)
{
    pr_info("Tomsk State University forever!\n");
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed\n", procfs_name);
}

module_init(procfs1_init);
module_exit(procfs1_exit);