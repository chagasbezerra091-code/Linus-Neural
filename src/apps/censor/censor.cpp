// SPDX-License-Identifier: Apache-2.0
/*
 * censor.cpp — Linus Neural Project
 *
 * Aplicativo de censura de linguagem: substitui palavras sensíveis por
 * alternativas neutras ou asteriscos.
 *
 * Copyright (c) 2025 Linus Neural Project
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>

using namespace std;

class Censor {
private:
    unordered_map<string, string> dictionary;

    string toLower(const string &s) {
        string r = s;
        transform(r.begin(), r.end(), r.begin(), ::tolower);
        return r;
    }

public:
    Censor() {
        // Dicionário de censura
        dictionary = {
            {"idiota", "pessoa confusa"},
            {"burro", "distraído"},
            {"palavrão", "***"},
            {"doido", "excêntrico"},
            {"boboca", "engraçado"}
        };
    }

    string filter(const string &input) {
        string output = input;
        for (auto &pair : dictionary) {
            string word = pair.first;
            string replacement = pair.second;

            size_t pos = 0;
            while ((pos = toLower(output).find(word, pos)) != string::npos) {
                output.replace(pos, word.size(), replacement);
                pos += replacement.size();
            }
        }
        return output;
    }
};

int main() {
    Censor censor;
    string text;

    cout << "=== 🧠 Linus Neural Project — Módulo de Censura ===\n";
    cout << "Digite uma frase (ou 'sair' para encerrar):\n\n";

    while (true) {
        cout << "> ";
        getline(cin, text);

        if (text == "sair")
            break;

        string clean = censor.filter(text);
        cout << "🔹 Versão limpa: " << clean << "\n\n";
    }

    cout << "=== Encerrado com segurança. ===\n";
    return 0;
}
