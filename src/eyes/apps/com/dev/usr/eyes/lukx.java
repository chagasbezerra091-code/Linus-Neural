// SPDX-License-Identifier: Apache-2.0
package com.dev.usr.eyes;

import java.util.Random;

/**
 * Simula um sensor de luz (luxímetro).
 * Retorna valores aleatórios de luminosidade ambiente.
 */
public class lux {
    private final Random rand = new Random();

    public int measureLight() {
        return rand.nextInt(1000); // valor entre 0 e 999 lux
    }
}
