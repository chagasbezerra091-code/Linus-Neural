// SPDX-License-Identifier: Apache-2.0 /**

eyes_driver.c - Experimental Eye Sensor Kernel Module

For the Linus Neural Project. This module creates a character device

/dev/eyes that simulates a simple eye-tracking / ambient light sensor.

It is intended for educational and development use only.

Copyright (c) 2025 Linus Neural Project */


#include <linux/module.h> #include <linux/kernel.h> #include <linux/init.h> #include <linux/fs.h> #include <linux/uaccess.h> #include <linux/device.h> #include <linux/mutex.h> #include <linux/random.h> #include <linux/slab.h> #include <linux/time.h>

#define DEVICE_NAME "eyes" #define CLASS_NAME  "sensornet"

static int    major_number; static struct class*  eyes_class  = NULL; static struct device* eyes_device = NULL; static DEFINE_MUTEX(eyes_mutex);

/* Simple runtime configuration exposed via write to device: / static unsigned int sample_interval_ms = 100; / how often sample updates (simulated) */ static bool simulate_blink = true;

/* Buffer to format sensor output for user-space */ #define EYES_BUF_SZ 256 static char eyes_buffer[EYES_BUF_SZ];

/* file operations prototypes / static int     eyes_open(struct inode, struct file*); static int     eyes_release(struct inode*, struct file*); static ssize_t eyes_read(struct file*, char*, size_t, loff_t*); static ssize_t eyes_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = { .open = eyes_open, .read = eyes_read, .write = eyes_write, .release = eyes_release, };

/* Helper: produce a simulated gaze coordinate (x,y) and brightness (lux) / struct eyes_sample { uint16_t x;     / 0..1920 / uint16_t y;     / 0..1080 / uint16_t lux;   / ambient light lux / bool blink;     / blink event / ktime_t ts;     / timestamp */ };

static void generate_sample(struct eyes_sample *s) { uint32_t rnd;

get_random_bytes(&rnd, sizeof(rnd));
s->x = (rnd % 1921); /* inclusive 0..1920 */
s->y = ((rnd >> 16) % 1081);

/* ambient light: 0..10000 lux typical range */
s->lux = (rnd % 10001);

/* blink: small probability depending on simulate_blink */
if (simulate_blink) {
    get_random_bytes(&rnd, sizeof(rnd));
    s->blink = ((rnd % 100) < 5); /* ~5% chance */
} else {
    s->blink = false;
}

s->ts = ktime_get();

}

static int eyes_open(struct inode *inodep, struct file *filep) { if (!mutex_trylock(&eyes_mutex)) { pr_alert("eyes_driver: device is busy\n"); return -EBUSY; } pr_info("eyes_driver: device opened\n"); return 0; }

static ssize_t eyes_read(struct file *filep, char *buffer, size_t len, loff_t *offset) { struct eyes_sample samp; int ret; size_t out_len;

/* Generate a fresh sample on each read */
generate_sample(&samp);

/* Format: timestamp_ms x y lux blink\n  e.g. 1617812345678 512 384 120 true\n */
snprintf(eyes_buffer, EYES_BUF_SZ, "%llu %u %u %u %s\n",
         (unsigned long long)ktime_to_ns(samp.ts) / 1000000ULL,
         (unsigned int)samp.x,
         (unsigned int)samp.y,
         (unsigned int)samp.lux,
         samp.blink ? "true" : "false");

out_len = strlen(eyes_buffer);
if (len < out_len) {
    /* user buffer too small */
    pr_warn("eyes_driver: user buffer too small: need %zu bytes\n", out_len);
    return -EINVAL;
}

ret = copy_to_user(buffer, eyes_buffer, out_len);
if (ret != 0) {
    pr_warn("eyes_driver: failed to copy %d bytes to user\n", ret);
    return -EFAULT;
}

pr_debug("eyes_driver: sample delivered: %s", eyes_buffer);
return out_len;

}

static ssize_t eyes_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) { char *kbuf; ssize_t rc = len;

/* Accept simple text commands to adjust simulated parameters:
 * e.g. "interval=200" or "blink=0" or "blink=1" 
 */
kbuf = kzalloc(len + 1, GFP_KERNEL);
if (!kbuf)
    return -ENOMEM;

if (copy_from_user(kbuf, buffer, len)) {
    kfree(kbuf);
    return -EFAULT;
}

/* parse basic commands */
if (strnstr(kbuf, "interval=", len)) {
    unsigned int v = 0;
    if (sscanf(kbuf, "interval=%u", &v) == 1 && v > 0)
        sample_interval_ms = v;
    pr_info("eyes_driver: set interval=%u ms\n", sample_interval_ms);
} else if (strnstr(kbuf, "blink=", len)) {
    unsigned int v = 0;
    if (sscanf(kbuf, "blink=%u", &v) == 1)
        simulate_blink = (v != 0);
    pr_info("eyes_driver: set blink=%u\n", simulate_blink);
} else {
    pr_info("eyes_driver: unknown command: %s\n", kbuf);
}

kfree(kbuf);
return rc;

}

static int eyes_release(struct inode *inodep, struct file *filep) { mutex_unlock(&eyes_mutex); pr_info("eyes_driver: device closed\n"); return 0; }

/* Module init/exit */ static int __init eyes_init(void) { pr_info("eyes_driver: initializing...\n");

major_number = register_chrdev(0, DEVICE_NAME, &fops);
if (major_number < 0) {
    pr_err("eyes_driver: failed to register char device\n");
    return major_number;
}

eyes_class = class_create(THIS_MODULE, CLASS_NAME);
if (IS_ERR(eyes_class)) {
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_err("eyes_driver: failed to create class\n");
    return PTR_ERR(eyes_class);
}

eyes_device = device_create(eyes_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
if (IS_ERR(eyes_device)) {
    class_destroy(eyes_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_err("eyes_driver: failed to create device\n");
    return PTR_ERR(eyes_device);
}

mutex_init(&eyes_mutex);
pr_info("eyes_driver: module loaded (major=%d)\n", major_number);
return 0;

}

static void __exit eyes_exit(void) { mutex_destroy(&eyes_mutex); device_destroy(eyes_class, MKDEV(major_number, 0)); class_unregister(eyes_class); class_destroy(eyes_class); unregister_chrdev(major_number, DEVICE_NAME); pr_info("eyes_driver: module removed\n"); }

MODULE_LICENSE("Apache-2.0"); MODULE_AUTHOR("Linus Neural Project"); MODULE_DESCRIPTION("Experimental eye/ambient sensor character driver (simulated samples)"); MODULE_VERSION("0.1");

module_init(eyes_init); module_exit(eyes_exit);
