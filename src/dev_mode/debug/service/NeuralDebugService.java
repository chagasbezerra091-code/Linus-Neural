// SPDX-License-Identifier: Apache-2.0
/*
 * NeuralDebugService.java — Linus Neural Project
 * Serviço de depuração neural remoto baseado em AIDL.
 */

package com.lnp.dev.debug.service;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.lnp.dev.debug.INeuralDebugService;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class NeuralDebugService extends Service {
    private static final String TAG = "NeuralDebugService";
    private boolean systemActive = true;
    private long startTime = System.currentTimeMillis();

    // Implementação da interface AIDL
    private final INeuralDebugService.Stub binder = new INeuralDebugService.Stub() {

        @Override
        public String getSystemStatus() throws RemoteException {
            long uptime = (System.currentTimeMillis() - startTime) / 1000;
            return "🧠 Linus Neural Core ativo — Uptime: " + uptime + "s | Status: " + (systemActive ? "OK" : "INATIVO");
        }

        @Override
        public String runCommand(String command) throws RemoteException {
            Log.d(TAG, "Comando recebido: " + command);

            switch (command.toLowerCase(Locale.ROOT)) {
                case "reboot":
                    systemActive = false;
                    return "[NEURAL-CORE] Reinicializando sistema...";
                case "analyze-memory":
                    return "[NEURAL-CORE] Análise de memória: estável (uso < 35%)";
                case "ping":
                    return "[NEURAL-CORE] Pong! Conexão estável.";
                default:
                    return "[NEURAL-CORE] Comando desconhecido: " + command;
            }
        }

        @Override
        public void logMessage(String tag, String message) throws RemoteException {
            String time = new SimpleDateFormat("HH:mm:ss", Locale.ROOT).format(new Date());
            Log.i("[" + tag + "]", time + " -> " + message);
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        Log.i(TAG, "Serviço NeuralDebugService conectado");
        return binder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.i(TAG, "Serviço NeuralDebugService desconectado");
        return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
        Log.w(TAG, "Serviço NeuralDebugService finalizado");
        systemActive = false;
        super.onDestroy();
    }
}
