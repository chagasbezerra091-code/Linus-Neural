// SPDX-License-Identifier: Apache-2.0
/*
 * neural_driver.c - Experimental Neural Interface Kernel Module
 *
 * Este módulo demonstra um driver de interface neural em nível de kernel.
 * Simula leitura e escrita de “sinais neurais”, apenas para estudo.
 *
 * Copyright (c) 2025 Linus Neural Project
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/random.h>

#define DEVICE_NAME "neural"
#define CLASS_NAME  "neuralnet"

static int    major_number;
static struct class*  neural_class  = NULL;
static struct device* neural_device = NULL;
static DEFINE_MUTEX(neural_mutex);

static char neural_buffer[256] = "Neural driver ativo.\n";

// Protótipos
static int     neural_open(struct inode*, struct file*);
static int     neural_release(struct inode*, struct file*);
static ssize_t neural_read(struct file*, char*, size_t, loff_t*);
static ssize_t neural_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = {
    .open = neural_open,
    .read = neural_read,
    .write = neural_write,
    .release = neural_release,
};

// Inicialização
static int __init neural_init(void) {
    printk(KERN_INFO "neural_driver: inicializando...\n");

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "neural_driver: falha ao registrar número de dispositivo\n");
        return major_number;
    }

    neural_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(neural_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(neural_class);
    }

    neural_device = device_create(neural_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(neural_device)) {
        class_destroy(neural_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(neural_device);
    }

    mutex_init(&neural_mutex);
    printk(KERN_INFO "neural_driver: módulo carregado! Major=%d\n", major_number);
    return 0;
}

static void __exit neural_exit(void) {
    mutex_destroy(&neural_mutex);
    device_destroy(neural_class, MKDEV(major_number, 0));
    class_unregister(neural_class);
    class_destroy(neural_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "neural_driver: módulo removido.\n");
}

// Operações
static int neural_open(struct inode *inodep, struct file *filep) {
    if (!mutex_trylock(&neural_mutex)) {
        printk(KERN_ALERT "neural_driver: dispositivo em uso\n");
        return -EBUSY;
    }
    printk(KERN_INFO "neural_driver: dispositivo aberto\n");
    return 0;
}

static ssize_t neural_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int noise;
    get_random_bytes(&noise, sizeof(noise));
    snprintf(neural_buffer, sizeof(neural_buffer),
             "[neural_driver] Atividade cerebral detectada: %d µV\n",
             (noise % 200) - 100);

    if (copy_to_user(buffer, neural_buffer, strlen(neural_buffer)) == 0) {
        printk(KERN_INFO "neural_driver: leitura enviada ao user-space\n");
        return strlen(neural_buffer);
    } else {
        printk(KERN_WARNING "neural_driver: falha ao enviar dados\n");
        return -EFAULT;
    }
}

static ssize_t neural_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    char input[64];
    size_t copy_len = min(len, sizeof(input) - 1);
    if (copy_from_user(input, buffer, copy_len))
        return -EFAULT;
    input[copy_len] = '\0';

    printk(KERN_INFO "neural_driver: comando recebido: %s\n", input);
    snprintf(neural_buffer, sizeof(neural_buffer),
             "[neural_driver] Comando neural processado: %s\n", input);
    return len;
}

static int neural_release(struct inode *inodep, struct file *filep) {
    mutex_unlock(&neural_mutex);
    printk(KERN_INFO "neural_driver: dispositivo fechado\n");
    return 0;
}

MODULE_LICENSE("Apache-2.0");
MODULE_AUTHOR("Linus Neural Project");
MODULE_DESCRIPTION("Driver experimental de interface neural com sinais simulados");
MODULE_VERSION("0.2");

module_init(neural_init);
module_exit(neural_exit);
