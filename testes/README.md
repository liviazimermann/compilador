# Programas de teste — T5 / T6

Cole o conteúdo de cada arquivo na IDE e clique em **Compilar** para ver a tabela
de símbolos, os avisos e o código BIP gerado (ou a mensagem de erro nos `erro_*`).

## T5
- `01_relacional.txt` — operadores relacionais → 0/1 (resultado é `booleano`)
- `02_se_simples.txt` — `se ( ) { } fim`
- `03_se_senao.txt` — `se ( ) { } senao { } fim`
- `04_enquanto.txt` — laço com teste no início
- `05_faca_enquanto.txt` — laço com teste no fim
- `06_para.txt` — laço com variável de controle
- `07_aninhamento.txt` — `para` › `enquanto` › `se`
- `08_logicos.txt` — `&&` e `||`
- `09_vetor.txt` — leitura/escrita de vetor com índice variável
- `13_negacao_unario.txt` — `!` e menos unário

## T6
- `10_subrotina_retorno.txt` — função com parâmetros e retorno
- `11_subrotina_void.txt` — sub-rotina `nulo`
- `12_chamada_aninhada.txt` — `f(f(x))`
- `14_programa_completo.txt` — função + `se/senao` + `leia` + chamada

## Erros esperados (análise semântica)
- `erro_rotina_inexistente.txt` → *Identificador 'coisa' nao declarado.*
- `erro_qtd_parametros.txt` → *Poucos argumentos para a funcao 'teste'.*
- `erro_tipo_parametro.txt` → *Tipo incompativel no parametro 0 da funcao 'f'.*
- `erro_var_nao_declarada.txt` → *Identificador 'y' nao declarado.*

## Observações sobre tipos
- O resultado de uma comparação/lógica é `booleano` — atribua a uma variável
  `booleano` (não `inteiro`).
- Sintaxe de vetor na declaração: `var v[5] : inteiro;`  acesso: `v[i]`.
- `dobro` é palavra reservada (tipo) — não use como nome de variável/função.
- `*`, `/`, `%` não geram código (o BIP não tem essas instruções).
