// SPDX-License-Identifier: Apache-2.0
/*
 * SafetyCore — Linus Neural Project
 * Watchdog de integridade e segurança.
 * Módulo apenas de leitura: verifica processos, permissões e integridade.
 */

#include "safety_core.h"
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>

static FILE *logf = NULL;

void safety_init(void) {
    logf = fopen("/tmp/safetycore.log", "a");
    if (!logf) logf = stderr;
    time_t t = time(NULL);
    fprintf(logf, "\n[SafetyCore] Iniciado em %s\n", ctime(&t));
}

void safety_log_event(SafetyEvent e) {
    const char *lvl = (e.level == SAFETY_ALERT) ? "ALERTA" :
                      (e.level == SAFETY_WARN) ? "AVISO" : "OK";
    fprintf(logf, "[%s] %s\n", lvl, e.message);
    fflush(logf);
}

/* Verifica se há processos suspeitos */
void safety_scan_processes(void) {
    FILE *fp = popen("ps -eo user,comm,pcpu --no-headers | head -n 20", "r");
    if (!fp) {
        safety_log_event((SafetyEvent){SAFETY_ALERT, "Falha ao listar processos"});
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "nc ") || strstr(line, "netcat") || strstr(line, "curl ")) {
            safety_log_event((SafetyEvent){SAFETY_WARN, "Processo potencialmente suspeito detectado"});
        }
    }
    pclose(fp);
}

/* Verifica permissões de arquivos sensíveis */
void safety_check_permissions(void) {
    struct stat st;
    if (stat("/etc/shadow", &st) == 0) {
        if ((st.st_mode & 0777) != 0600) {
            safety_log_event((SafetyEvent){SAFETY_ALERT, "/etc/shadow com permissões incorretas!"});
        }
    }
    if (stat("/system", &st) == 0) {
        if ((st.st_mode & S_IWOTH)) {
            safety_log_event((SafetyEvent){SAFETY_ALERT, "/system gravável por outros!"});
        }
    }
}

/* Verifica integridade básica de arquivos importantes */
void safety_scan_integrity(void) {
    FILE *fp = fopen("safety_rules.conf", "r");
    if (!fp) {
        safety_log_event((SafetyEvent){SAFETY_WARN, "Não foi possível abrir safety_rules.conf"});
        return;
    }
    char path[256];
    struct stat st;
    while (fgets(path, sizeof(path), fp)) {
        path[strcspn(path, "\n")] = 0;
        if (stat(path, &st) != 0) {
            char msg[300];
            snprintf(msg, sizeof(msg), "Arquivo ausente: %s", path);
            safety_log_event((SafetyEvent){SAFETY_WARN, msg});
        }
    }
    fclose(fp);
}

void safety_shutdown(void) {
    time_t t = time(NULL);
    fprintf(logf, "[SafetyCore] Finalizado em %s\n", ctime(&t));
    if (logf != stderr) fclose(logf);
}

int main(void) {
    safety_init();
    safety_scan_processes();
    safety_check_permissions();
    safety_scan_integrity();
    safety_shutdown();
    return 0;
}
