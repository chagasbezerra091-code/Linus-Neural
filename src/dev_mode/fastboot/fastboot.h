// SPDX-License-Identifier: Apache-2.0
/*
 * fastboot.h — Linus Neural Project
 *
 * Cabeçalho do módulo Fastboot (modo desenvolvedor)
 *
 * Copyright (c) 2025 Linus Neural Project
 */

#ifndef LNP_FASTBOOT_H
#define LNP_FASTBOOT_H

#include <string>
#include <map>

namespace lnp_dev {

class Fastboot {
private:
    std::map<std::string, std::string> commands;
    bool connected;

public:
    Fastboot();
    void connect();
    void disconnect();
    void executeCommand(const std::string &cmd);
    void showHelp();
    bool isConnected() const { return connected; }
};

} // namespace lnp_dev

#endif // LNP_FASTBOOT_H
