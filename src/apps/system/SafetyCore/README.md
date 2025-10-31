# SafetyCore

**SafetyCore** é o módulo de segurança e integridade do Linus Neural Project.

## Funções principais
- Verifica processos suspeitos.
- Confere permissões de arquivos críticos.
- Checa integridade de arquivos definidos em `safety_rules.conf`.
- Registra alertas em `/tmp/safetycore.log`.

## Uso
```bash
make
sudo ./safety_core
cat /tmp/safetycore.log
