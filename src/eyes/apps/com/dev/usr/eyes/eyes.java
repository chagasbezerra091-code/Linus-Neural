// SPDX-License-Identifier: Apache-2.0
package com.dev.usr.eyes;

import java.util.Random;

/**
 * Classe principal do sistema de visão do Linus Neural Project.
 * Simula captura de imagem e envia dados brutos para a IA interpretar.
 */
public class eyes {
    private boolean active;
    private final Random rand;
    private final ia neuralIA;
    private final lux lightSensor;

    public eyes() {
        this.active = false;
        this.rand = new Random();
        this.neuralIA = new ia();
        this.lightSensor = new lux();
    }

    public void startVision() {
        System.out.println("[EYES] Inicializando visão neural...");
        active = true;
        for (int i = 0; i < 5; i++) {
            captureFrame();
            try { Thread.sleep(1000); } catch (InterruptedException ignored) {}
        }
        System.out.println("[EYES] Encerrando visão neural.");
    }

    private void captureFrame() {
        if (!active) return;
        int light = lightSensor.measureLight();
        String frame = "[EYES] Captura de quadro #" + (rand.nextInt(9000) + 1000)
                + " | Luminosidade=" + light + " lux";
        System.out.println(frame);

        String analysis = neuralIA.processFrame(frame, light);
        System.out.println("[EYES] Interpretação IA → " + analysis);
    }

    public void stopVision() {
        this.active = false;
    }
}
