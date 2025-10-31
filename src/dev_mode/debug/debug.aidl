// SPDX-License-Identifier: Apache-2.0
/*
 * debug.aidl — Linus Neural Project
 * Interface de comunicação AIDL para depuração remota
 */

package com.lnp.dev.debug;

// Interface de debug remoto neural
interface INeuralDebugService {
    /**
     * Retorna o status atual do sistema neural.
     */
    String getSystemStatus();

    /**
     * Envia um comando de depuração remoto.
     * @param command O comando a ser executado.
     * @return Resultado textual da execução.
     */
    String runCommand(String command);

    /**
     * Registra uma mensagem no log neural.
     * @param tag Categoria do log.
     * @param message Texto da mensagem.
     */
    void logMessage(String tag, String message);
}
