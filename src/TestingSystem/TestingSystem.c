// SPDX-License-Identifier: Apache-2.0
/*
 * TestingSystem.c
 *
 * Single-file system tester for Linux (designed to run as root).
 * Performs read-only checks: CPU, memory, disk, kernel modules,
 * dmesg tail, network interfaces, common tools availability,
 * presence of specific device nodes (e.g. /dev/neural), etc.
 *
 * Compile:
 *   gcc TestingSystem.c -o test_system
 * Run (recommended as root for full checks):
 *   sudo ./test_system
 *
 * Copyright (c) 2025 Linus Neural Project - TestingSystem
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <time.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <dirent.h>

#define BUF_SIZE 4096

/* Utility: run a shell command and capture first N bytes of output */
static int run_cmd(const char *cmd, char *out, size_t out_len, int max_lines) {
    FILE *fp;
    size_t written = 0;
    int lines = 0;

    if (!cmd || !out) return -1;
    fp = popen(cmd, "r");
    if (!fp) {
        snprintf(out, out_len, "ERROR: popen failed (%s)", strerror(errno));
        return -1;
    }

    while (fgets(out + written, (int)(out_len - written), fp) != NULL) {
        written = strlen(out);
        lines++;
        if (max_lines > 0 && lines >= max_lines) break;
        if (written >= out_len - 2) break;
    }
    pclose(fp);
    return 0;
}

/* Utility: check if file exists & is readable */
static int file_exists_readable(const char *path) {
    return (access(path, R_OK) == 0);
}

/* Print header */
static void print_header(const char *title) {
    time_t t = time(NULL);
    char ts[64];
    strftime(ts, sizeof(ts), "%F %T", localtime(&t));
    printf("\n=== %s ===\nTime: %s\n\n", title, ts);
}

/* 1. Check running user */
static void check_user(void) {
    print_header("User / Permissions Check");
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    printf("Effective UID: %d\n", uid);
    if (pw) printf("User name: %s\n", pw->pw_name);
    if (uid == 0) {
        printf("You are running as root. Full checks will run.\n");
    } else {
        printf("Not running as root. Some checks will be limited.\n");
    }
}

/* 2. CPU info (reads /proc/cpuinfo first processor summary) */
static void check_cpu(void) {
    print_header("CPU Info");
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (!f) {
        printf("Unable to open /proc/cpuinfo: %s\n", strerror(errno));
        return;
    }
    char line[512];
    char model[256] = "<unknown>";
    int cores = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "model name", 10) == 0 || strncmp(line, "Processor", 9) == 0) {
            char *p = strchr(line, ':');
            if (p) {
                while (*++p == ' ') ; /* skip space */
                strncpy(model, p, sizeof(model)-1);
                /* trim newline */
                char *nl = strchr(model, '\n');
                if (nl) *nl = '\0';
            }
        }
        if (strncmp(line, "processor", 9) == 0 || strncmp(line, "cpu cores", 9) == 0) {
            cores++;
        }
    }
    fclose(f);
    if (cores == 0) { /* fallback: use sysconf */
        cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    }
    printf("Model: %s\n", model);
    printf("Cores: %d\n", cores);
}

/* 3. Memory info (sysinfo + /proc/meminfo brief) */
static void check_memory(void) {
    print_header("Memory Info");
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        printf("Total RAM: %lu MB\n", si.totalram / 1024 / 1024);
        printf("Free RAM:  %lu MB\n", si.freeram / 1024 / 1024);
        printf("Uptime:    %ld seconds\n", si.uptime);
    } else {
        printf("sysinfo() failed: %s\n", strerror(errno));
    }

    /* show MemTotal and MemAvailable from /proc/meminfo (if available) */
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) {
        printf("Unable to open /proc/meminfo: %s\n", strerror(errno));
        return;
    }
    char key[128];
    unsigned long val;
    char unit[32];
    int shown = 0;
    while (fscanf(f, "%127s %lu %31s\n", key, &val, unit) == 3) {
        if (strcmp(key, "MemTotal:") == 0 || strcmp(key, "MemAvailable:") == 0) {
            printf("%s %lu %s\n", key, val, unit);
            shown++;
            if (shown >= 2) break;
        }
    }
    fclose(f);
}

/* 4. Disk usage (statvfs on / ) */
static void check_disk(void) {
    print_header("Disk & Filesystem Info");
    struct statvfs sv;
    if (statvfs("/", &sv) == 0) {
        unsigned long total = (sv.f_frsize * sv.f_blocks) / 1024 / 1024;
        unsigned long free  = (sv.f_frsize * sv.f_bfree) / 1024 / 1024;
        unsigned long used  = total - free;
        printf("/ - total: %lu MB, used: %lu MB, free: %lu MB\n", total, used, free);
    } else {
        printf("statvfs('/') failed: %s\n", strerror(errno));
    }

    /* Show top mounted filesystems (first 6 lines of /proc/mounts) */
    char buf[BUF_SIZE] = {0};
    if (run_cmd("head -n 6 /proc/mounts", buf, sizeof(buf), 0) == 0) {
        printf("\nMounted filesystems (top 6):\n%s", buf);
    }
}

/* 5. Kernel modules (lsmod) */
static void check_lsmod(void) {
    print_header("Loaded Kernel Modules (lsmod top 20)");
    char buf[BUF_SIZE] = {0};
    if (run_cmd("lsmod | head -n 20", buf, sizeof(buf), 20) == 0) {
        printf("%s", buf);
    } else {
        printf("Failed to run lsmod\n");
    }
}

/* 6. Tail of dmesg (last 10 lines) */
static void check_dmesg_tail(void) {
    print_header("dmesg (last 10 lines)");
    char buf[BUF_SIZE] = {0};
    if (run_cmd("dmesg -T | tail -n 10", buf, sizeof(buf), 10) == 0) {
        printf("%s", buf);
    } else {
        printf("Failed to run dmesg\n");
    }
}

/* 7. Network interfaces & addresses */
static void check_network(void) {
    print_header("Network Interfaces & Addresses");
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        printf("getifaddrs failed: %s\n", strerror(errno));
        return;
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) {
            char host[NI_MAXHOST];
            int s = getnameinfo(ifa->ifa_addr,
                                (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s == 0) {
                printf("%s\t%s\t%s\n", ifa->ifa_name,
                       (family == AF_INET) ? "IPv4" : "IPv6",
                       host);
            }
        }
    }
    freeifaddrs(ifaddr);

    /* Check network tools presence */
    char buf[256];
    printf("\nCommand checks:\n");
    if (run_cmd("which ip || which ifconfig", buf, sizeof(buf), 1) == 0 && strlen(buf) > 0) {
        printf("Networking tool: %s", buf);
    } else {
        printf("No ip/ifconfig found in PATH\n");
    }
}

/* 8. Check for presence of common binaries (fastboot, adb, java, gcc) */
static void check_binaries(void) {
    print_header("Binary Availability");
    const char *bins[] = {"fastboot", "adb", "java", "javac", "gcc", "make", "python3", NULL};
    char buf[256];
    for (int i = 0; bins[i]; ++i) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "command -v %s >/dev/null 2>&1 && echo FOUND || echo MISSING", bins[i]);
        if (run_cmd(cmd, buf, sizeof(buf), 1) == 0) {
            /* trim newline */
            char *nl = strchr(buf, '\n'); if (nl) *nl = '\0';
            printf("%-8s : %s\n", bins[i], buf);
        } else {
            printf("%-8s : check failed\n", bins[i]);
        }
    }
}

/* 9. Check for device nodes that make sense for this project */
static void check_dev_nodes(void) {
    print_header("Device Nodes (expected examples)");
    const char *devs[] = {"/dev/neural", "/dev/eyes", "/dev/touchscreen", "/dev/input/event0", "/dev/null", NULL};
    struct stat st;
    for (int i = 0; devs[i]; ++i) {
        if (stat(devs[i], &st) == 0) {
            printf("%-20s : EXISTS (mode=0%o)\n", devs[i], st.st_mode & 0777);
        } else {
            printf("%-20s : MISSING (errno=%d %s)\n", devs[i], errno, strerror(errno));
        }
    }
}

/* 10. Simple kernel version & uname */
static void check_uname(void) {
    print_header("Kernel & System Info (uname)");
    char buf[BUF_SIZE] = {0};
    if (run_cmd("uname -a", buf, sizeof(buf), 1) == 0) {
        printf("%s", buf);
    } else {
        printf("uname failed\n");
    }

    /* Also show /etc/os-release if exists */
    if (file_exists_readable("/etc/os-release")) {
        char out[512] = {0};
        if (run_cmd("cat /etc/os-release | sed -n '1,6p'", out, sizeof(out), 6) == 0) {
            printf("\nOS release (top lines):\n%s", out);
        }
    }
}

/* 11. Check processes (top few lines of ps aux) */
static void check_processes(void) {
    print_header("Running Processes (top 10 by cpu)");
    char buf[BUF_SIZE] = {0};
    if (run_cmd("ps aux --sort=-%cpu | head -n 11", buf, sizeof(buf), 11) == 0) {
        printf("%s", buf);
    } else {
        printf("ps failed\n");
    }
}

/* 12. Check /proc entries for potential kernel alerts (oom, errors) */
static void check_proc_status(void) {
    print_header("/proc/sys and kernel alerts");
    char buf[BUF_SIZE] = {0};
    if (file_exists_readable("/proc/sys/vm/overcommit_memory")) {
        if (run_cmd("cat /proc/sys/vm/overcommit_memory", buf, sizeof(buf), 1) == 0)
            printf("vm.overcommit_memory = %s", buf);
    }
    /* check last kernel OOPS or panic lines in dmesg */
    if (run_cmd("dmesg | egrep -i 'oom|panic|oops' | tail -n 10", buf, sizeof(buf), 10) == 0) {
        if (strlen(buf) > 0)
            printf("\nRecent kernel warnings (oom/panic/oops):\n%s", buf);
        else
            printf("\nNo recent kernel OOM/PANIC/OOPS messages found in dmesg tail.\n");
    }
}

/* 13. Quick check for mounted loop devices (useful in test envs) */
static void check_loop_devices(void) {
    print_header("Loop Devices (ls /dev/loop*)");
    char buf[BUF_SIZE] = {0};
    if (run_cmd("ls -1 /dev/loop* 2>/dev/null | sed -n '1,10p'", buf, sizeof(buf), 10) == 0) {
        if (strlen(buf) > 0)
            printf("%s", buf);
        else
            printf("No loop devices found or not accessible.\n");
    }
}

/* 14. Basic network connectivity test (ping a reliable host) */
static void check_connectivity(void) {
    print_header("Network Connectivity Test (ping)");
    char buf[BUF_SIZE] = {0};
    /* Use a short ping to 8.8.8.8 but avoid long waits */
    if (run_cmd("ping -c 2 -W 1 8.8.8.8 2>/dev/null | tail -n 3", buf, sizeof(buf), 3) == 0) {
        if (strstr(buf, "0% packet loss") || strstr(buf, "rtt")) {
            printf("Ping success summary:\n%s", buf);
        } else if (strlen(buf) > 0) {
            printf("Ping attempt output:\n%s", buf);
        } else {
            printf("Ping command produced no output (maybe blocked by firewall).\n");
        }
    } else {
        printf("Ping failed to execute.\n");
    }
}

/* 15. Check for Java environment (useful for apps tests) */
static void check_java_env(void) {
    print_header("Java Environment Check");
    char buf[BUF_SIZE] = {0};
    if (run_cmd("java -version 2>&1 | head -n 1", buf, sizeof(buf), 1) == 0) {
        if (strlen(buf) > 0) printf("%s", buf);
    } else {
        printf("java not found or failed to run.\n");
    }
    if (run_cmd("javac -version 2>&1 | head -n 1", buf, sizeof(buf), 1) == 0) {
        if (strlen(buf) > 0) printf("%s", buf);
    } else {
        printf("javac not found or failed to run.\n");
    }
}

/* 16. Quick sanity test for directory permissions in /tmp */
static void check_tmp_permissions(void) {
    print_header("Temporary Directory Test (/tmp)");
    const char *testfile = "/tmp/lnp_testing_system_check.tmp";
    FILE *f = fopen(testfile, "w");
    if (f) {
        fprintf(f, "lnp test\n");
        fclose(f);
        printf("Wrote and removed %s — OK\n", testfile);
        unlink(testfile);
    } else {
        printf("Failed to write to %s: %s\n", testfile, strerror(errno));
    }
}

/* Driver / service presence heuristic checks (non-invasive) */
static void check_services_drivers(void) {
    print_header("Heuristic: Services & Drivers (non-invasive)");
    /* Check for systemd units relevant to LNP if present */
    char buf[BUF_SIZE] = {0};
    if (run_cmd("systemctl list-units --type=service --no-pager --all | egrep 'neural|lnp|eyes|fastboot' | head -n 20'", buf, sizeof(buf), 20) == 0) {
        if (strlen(buf) > 0) {
            printf("Potential system services related to project:\n%s", buf);
        } else {
            printf("No obvious LNP-related system services found via systemctl.\n");
        }
    } else {
        printf("systemctl not available or failed.\n");
    }

    /* Check /proc/devices for character devices list */
    if (file_exists_readable("/proc/devices")) {
        if (run_cmd("grep -i neural /proc/devices || true", buf, sizeof(buf), 5) == 0 && strlen(buf) > 0)
            printf("/proc/devices mentions:\n%s", buf);
        else
            printf("/proc/devices contains no 'neural' entry (expected in many systems).\n");
    }
}

/* Main: orchestrate checks */
int main(int argc, char **argv) {
    printf("=== Linus Neural Project — TestingSystem (single-file) ===\n");
    printf("Note: this tool performs read-only checks and light commands. It is safe,\n");
    printf("but running as root allows more complete information. Proceeding...\n");

    check_user();
    check_uname();
    check_cpu();
    check_memory();
    check_disk();
    check_lsmod();
    check_dmesg_tail();
    check_network();
    check_binaries();
    check_dev_nodes();
    check_processes();
    check_proc_status();
    check_loop_devices();
    check_connectivity();
    check_java_env();
    check_tmp_permissions();
    check_services_drivers();

    printf("\n=== TestingSystem completed. Review output above for any anomalies. ===\n");
    return 0;
}
