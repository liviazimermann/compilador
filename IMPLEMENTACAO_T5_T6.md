# Geração de Código BIP — Trabalhos T5 e T6

Geração de código para o processador **BIP** (conjunto de instruções do BIPIDE),
construída **sobre a base de análise semântica** (tabela de símbolos, escopos e
verificação de tipos) já existente no projeto.

| Arquivo | Conteúdo |
|---|---|
| `GALS_M3_T5_T6.gals` | Gramática (léxico + sintático) com as ações de análise **e** de geração de código |
| `IDE_Compilador/Semantico.cpp` / `Semantico.h` | Análise semântica + geração de código BIP |
| `IDE_Compilador/SemanticTable.h` | Tabela de compatibilidade de tipos |
| `testes/` | Programas de teste para cada requisito |

> Esta implementação parte da base do analisador semântico (tabela de símbolos
> com escopos, verificação de tipos, avisos) e da gramática **sem ambiguidade**.
> As ações de análise (`#1`–`#27`) foram mantidas; as de **geração de código**
> foram estendidas para cobrir T5 e T6.

---

## ⚙️ Como regenerar e compilar (IMPORTANTE)

A gramática foi estendida (novo token `retorne`, novos não-terminais e ações).
É **obrigatório regerar** o analisador léxico/sintático com o GALS:

1. Abra `GALS_M3_T5_T6.gals` no **GALS** e gere os analisadores em **C++**
   (atualiza `Lexico.*`, `Sintatico.*`, `Constants.*`).
2. **Mantenha `Semantico.cpp` / `Semantico.h` / `SemanticTable.h` deste projeto**
   (o GALS sobrescreve o `Semantico.cpp` com um *stub*; restaure a versão do
   projeto — mesmo fluxo dos commits anteriores).
3. Abra no **Qt Creator** e compile. A IDE mostra a tabela de símbolos, os
   avisos semânticos e o **código Assembly BIP** gerado.

> Validado de ponta a ponta: a gramática é SLR sem conflitos e todos os
> programas de `testes/` geram o código BIP esperado.

---

## ✅ T5 — Geração de Código II

| Requisito | Ações de codegen | Teste |
|---|---|---|
| Operações relacionais (`== != > < >= <=`) | `#41`–`#46`, `#81` | `01_relacional.txt` |
| Desvio condicional simples (`se … fim`) | `#90`, `#91` | `02_se_simples.txt` |
| Desvio condicional composto (`se … senao … fim`) | `#90`, `#93`, `#94` | `03_se_senao.txt` |
| Laço `enquanto` (teste no início) | `#95`, `#96`, `#97` | `04_enquanto.txt` |
| Laço `faca … enquanto` (teste no fim) | `#98`, `#99` | `05_faca_enquanto.txt` |
| Laço `para` (variável de controle) | `#100`–`#103` | `06_para.txt` |
| Aninhamentos | pilha de rótulos `pilhaCtrl` | `07_aninhamento.txt` |

Também cobertos: `&&`/`||` (`#82`–`#85`), `!` (`#74`), menos unário (`#75`),
literais booleanos (`#71`/`#72`).

### Comparação → 0/1 (relacional)

Os desvios do BIP usam o STATUS (Z/N) da última operação da ULA. Para `a < b`:

```
... ; (a no ACC)
STO 1003          ; salva esquerda            (#41–#46)
... ; (b no ACC)
STO 1004 ; LD 1003 ; SUB 1004                  (#81)
BLT REL0 ; LDI 0 ; JMP REL1 ; REL0: LDI 1 ; REL1:
```

Como `LDI`/`LD` não afetam o STATUS, antes de cada desvio condicional emite-se
`ADDI 0` para refletir o booleano (0/1) do ACC.

---

## ✅ T6 — Geração de Código III

| Requisito | Onde | Teste |
|---|---|---|
| Chamada de sub-rotinas (rótulos + `CALL`) | `#4` (rótulo `_func`), `#25`/`#26` (`CALL`) | `10`, `11` |
| Passagem de parâmetros (por cópia) | `#23` (`STO funcao_param`) | `10_subrotina_retorno.txt` |
| Retorno de funções (em expressões/atribuições) | `#33` (`retorne`), `#5` (`RETURN`) | `10`, `14` |
| Compatibilidade (tipo, ordem, quantidade) | `#23`, `#25`, `#26` (análise semântica) | `erro_*` |
| Mensagens de erro/aviso | `SemanticError` + `getAvisos()` | `erro_*` |

### Convenções de sub-rotina

- Cada parâmetro vira a célula `funcao_param` em `.data` (qualificada, evita
  colisão entre rotinas). Dentro do corpo, o nome do parâmetro é resolvido para
  essa célula (`resolveCelula`).
- O **chamador** copia cada argumento (`STO funcao_param`) antes do `CALL`.
- O **retorno** vai no `ACC` (`retorne <expr>;` → avalia e `RETURN`), permitindo
  uso em expressões e atribuições.
- O programa principal é emitido primeiro (rótulo `_PRINCIPAL`, termina em
  `HLT`); as sub-rotinas vêm depois, alcançadas apenas por `CALL`.

### Mensagens (já vindas da análise semântica)

```
Identificador 'coisa' nao declarado.
Poucos argumentos para a funcao 'teste'.
Muitos argumentos para a funcao 'teste'.
Tipo incompativel no parametro N da funcao 'f'.
Atribuicao invalida: tipo incompativel para 'r'.
```

---

## 🧠 Mapa de memória auxiliar

| Endereço | Uso |
|---|---|
| `1000` / `1001` | operandos aritméticos (com otimização *peephole* em `ADD/ADDI/SUB/SUBI`) |
| `1002` | índice de vetor (geral / lado direito) |
| `1003` / `1004` | operandos relacionais |
| `1005` / `1006` | `&&` (E lógico) |
| `1007` / `1008` | `\|\|` (OU lógico) |
| `1009` | índice do vetor no lado esquerdo da atribuição |
| `1010` | menos unário |
| `$indr` / `$in_port` / `$out_port` | índice de vetor / E/S |

---

## ⚠️ Limitações conhecidas

- **`*`, `/`, `%` não são gerados**: o BIP não possui multiplicação/divisão/resto
  (apenas `ADD/SUB`). A análise semântica valida os tipos, mas não há instrução
  para gerar — fora do escopo de T5/T6.
- **Recursão**: parâmetros usam células globais (sem pilha de ativação), logo
  recursão reentrante no mesmo parâmetro não é suportada.
- **Referência adiantada de função**: a função deve ser definida antes de ser
  chamada (análise em passagem única).
