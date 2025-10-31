// SPDX-License-Identifier: Apache-2.0
/*
 * fastboot.cc — Linus Neural Project
 *
 * Simulação de interface Fastboot (modo desenvolvedor)
 * Permite executar comandos básicos de manutenção e depuração.
 *
 * Copyright (c) 2025 Linus Neural Project
 */

#include "fastboot.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
using namespace lnp_dev;

Fastboot::Fastboot() : connected(false) {
    // Lista de comandos simulados
    commands = {
        {"flash", "Simula gravação de partição."},
        {"reboot", "Reinicia o sistema neural."},
        {"devices", "Lista dispositivos conectados."},
        {"getvar", "Mostra variáveis do sistema."},
        {"help", "Mostra esta lista de comandos."}
    };
}

void Fastboot::connect() {
    if (connected) {
        cout << "[FASTBOOT] Já conectado.\n";
        return;
    }
    cout << "[FASTBOOT] Conectando dispositivo..." << endl;
    this_thread::sleep_for(chrono::milliseconds(1000));
    connected = true;
    cout << "[FASTBOOT] Conexão estabelecida com sucesso!\n";
}

void Fastboot::disconnect() {
    if (!connected) {
        cout << "[FASTBOOT] Nenhum dispositivo conectado.\n";
        return;
    }
    cout << "[FASTBOOT] Desconectando..." << endl;
    this_thread::sleep_for(chrono::milliseconds(500));
    connected = false;
    cout << "[FASTBOOT] Dispositivo removido.\n";
}

void Fastboot::executeCommand(const string &cmd) {
    if (!connected) {
        cout << "[FASTBOOT] Nenhum dispositivo conectado. Use connect().\n";
        return;
    }

    string command = cmd;
    if (commands.find(command) == commands.end()) {
        cout << "[FASTBOOT] Comando desconhecido: " << command << "\n";
        cout << "Use 'help' para ver os comandos disponíveis.\n";
        return;
    }

    cout << "[FASTBOOT] Executando comando: " << command << "...\n";
    this_thread::sleep_for(chrono::milliseconds(700));

    if (command == "flash") {
        cout << "[FASTBOOT] Flash simulado concluído com sucesso.\n";
    } else if (command == "reboot") {
        cout << "[FASTBOOT] Reiniciando sistema neural...\n";
        this_thread::sleep_for(chrono::milliseconds(1000));
        cout << "[FASTBOOT] Sistema reiniciado.\n";
    } else if (command == "devices") {
        cout << "[FASTBOOT] Dispositivo detectado: LNP_DEV_ARM64_001\n";
    } else if (command == "getvar") {
        cout << "[FASTBOOT] version-bootloader: 1.0-neural\n";
        cout << "[FASTBOOT] product: LinusNeuralDevice\n";
    } else if (command == "help") {
        showHelp();
    }
}

void Fastboot::showHelp() {
    cout << "\n=== Comandos disponíveis no modo Fastboot ===\n";
    for (auto &pair : commands) {
        cout << "  • " << pair.first << " — " << pair.second << "\n";
    }
    cout << "============================================\n";
}
