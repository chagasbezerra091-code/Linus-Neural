// SPDX-License-Identifier: Apache-2.0
package com.dev.usr.eyes;

import java.util.Random;

/**
 * Classe responsável por processar quadros e interpretar padrões visuais.
 * Usa simulação de IA neural para "reconhecer" objetos.
 */
public class ia {
    private final Random rand = new Random();

    private final String[] objects = {
        "humano", "cachorro", "computador", "árvore", "celular", "luz forte", "movimento rápido"
    };

    public String processFrame(String frame, int lux) {
        String obj = objects[rand.nextInt(objects.length)];
        String clarity = (lux < 100) ? "baixa luz" :
                         (lux < 500) ? "luz moderada" : "alta luz";

        return "Objeto: " + obj + " | Condição: " + clarity;
    }
}
