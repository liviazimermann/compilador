#include "Semantico.h"
#include "Constants.h"

#include <sstream>
#include <iostream>

using namespace std;

Semantico::Semantico()
    : tipoAtual(-1), tipoAtualFuncao(-1), contadorEscopo(0), contadorParametros(0),
      ultimoAcessoVetor(false), atribuicaoVetor(false), dentroDeclaracaoVetor(false)
{
    // Escopo global sempre na pilha
    pilhaEscopos.push("global");
}

// Ações semanticas 
void Semantico::executeAction(int action, const Token *token)
{

    switch (action)
    {
    // ── #1: Registra o tipo atual (vem de <tipo> ou <tipo_retorno>) ──────────
    case 1:
    {
        tipoAtual = tokenParaTipo(token);
        
        if (!variaveisSemTipo.empty()) {
            for (int idx : variaveisSemTipo) {
                tabelaSimbolos[idx].tipo = tipoAtual;
            }
            variaveisSemTipo.clear();
        }
        break;
    }

    // ── #2: Insere VARIÁVEL escalar na tabela ────────────────────────────────
    case 2:
    {
        inserirSimbolo(token->getLexeme(), MOD_VARIAVEL, token->getPosition());
        variaveisSemTipo.push_back(tabelaSimbolos.size() - 1);
        break;
    }

    // ── #3: Insere VETOR na tabela ───────────────────────────────────────────
    case 3:
    {
        inserirSimbolo(token->getLexeme(), MOD_VETOR, token->getPosition());
        variaveisSemTipo.push_back(tabelaSimbolos.size() - 1);
        dentroDeclaracaoVetor = true;
        break;
    }

    // ── #4: Insere FUNÇÃO na tabela e abre escopo ────────────────────────────
    case 4:
    {
        string idFunc = token->getLexeme();
        
        inserirSimbolo(idFunc, MOD_FUNCAO, token->getPosition());
        tipoAtualFuncao = tipoAtual;
        contadorParametros = 0;
        
        nomeAtualFuncao = idFunc;
        // Abre escopo da função
        pilhaEscopos.push(idFunc);
        // Codegen: corpo da sub-rotina vai para o buffer de sub-rotinas
        emFuncao = true;
        emitir("_" + idFunc + ":");
        break;
    }

    // ── #5: Fecha escopo da função ───────────────────────────────────────────
    case 5:
        emitir("RETURN");        // retorno implícito (segurança)
        emFuncao = false;
        verificarNaoUsadas();
        if (pilhaEscopos.size() > 1)
            pilhaEscopos.pop();
        break;

    // ── #6: Insere PARÂMETRO escalar na tabela ───────────────────────────────
    case 6:
    {
        inserirSimbolo(token->getLexeme(), MOD_PARAMETRO, token->getPosition());
        variaveisSemTipo.push_back(tabelaSimbolos.size() - 1);
        tabelaSimbolos.back().numeroParametro = contadorParametros++;
        break;
    }

    // ── #7: Insere PARÂMETRO VETOR na tabela ─────────────────────────────────
    case 7:
    {
        inserirSimbolo(token->getLexeme(), MOD_PARAM_VETOR, token->getPosition());
        variaveisSemTipo.push_back(tabelaSimbolos.size() - 1);
        tabelaSimbolos.back().numeroParametro = contadorParametros++;
        dentroDeclaracaoVetor = true;
        break;
    }

    // ── #8: Abre escopo se/enquanto/para/faca ──────────────────────
    case 8:
    {
        ostringstream oss;
        oss << escopoAtual() << "_bloco" << (++contadorEscopo);
        pilhaEscopos.push(oss.str());
        break;
    }

    // ── #9: Fecha escopo se/enquanto/para/faca ─────────────────────────────
    case 9:
        verificarNaoUsadas();
        if (pilhaEscopos.size() > 1)
            pilhaEscopos.pop();
        break;

    // ── #20: Salva o operador da expressão ───────────────────────────────────
    case 20:
        pilhaOperadores.push(token->getId());
        break;

    // ── #19: Valida tipo do índice do vetor ──────────────────────────────────
    case 19:
    {
        if (pilhaTipos.empty()) break;
        int tipoIndice = pilhaTipos.top();
        pilhaTipos.pop();
        if (tipoIndice != SemanticTable::INT) {
            throw SemanticError("O indice do vetor deve ser um valor inteiro.", token->getPosition());
        }
        break;
    }

    // ── #18: Salva o nome do identificador lido ──────────────────────────────
    case 18:
        pilhaNomes.push(token->getLexeme());
        ultimoAcessoChamada = false;   // novo acesso: limpa flag de retorno de chamada
        break;

    // ── #16: Salva o lado esquerdo da atribuição ─────────────────────────────
    case 16:
    {
        pilhaAtribuicoes.push(idAtribuicao);
        Simbolo* s = buscarSimbolo(idAtribuicao);
        if (s != nullptr) {
            s->qtdUsos--;
            s->usada = (s->qtdUsos > 0);
        }
        if (!pilhaTipos.empty()) pilhaTipos.pop();
        
        string msgParaRemover = "AVISO: Variavel '" + idAtribuicao + "' usada sem inicializar (lixo de memoria).";
        for (auto it = avisos.rbegin(); it != avisos.rend(); ++it) {
            if (*it == msgParaRemover) {
                avisos.erase(std::next(it).base());
                break;
            }
        }
        
        break;
    }

    // ── #10: Verifica se identificador está declarado; empilha tipo ──────────
    case 10:
    {
        if (pilhaNomes.empty()) break;
        string nome = pilhaNomes.top();
        pilhaNomes.pop();

        Simbolo* s = buscarSimbolo(nome);

        if (s == nullptr) {
            ostringstream msg;
            msg << "Identificador '" << nome << "' nao declarado.";
            throw SemanticError(msg.str(), token->getPosition());
        }

        if (!s->inicializado &&
            (s->modalidade == MOD_VARIAVEL || s->modalidade == MOD_PARAMETRO ||
             s->modalidade == MOD_VETOR    || s->modalidade == MOD_PARAM_VETOR))
        {
            ostringstream msg;
            msg << "AVISO: Variavel '" << nome << "' usada sem inicializar (lixo de memoria).";
            avisos.push_back(msg.str());
        }

        s->qtdUsos++;
        s->usada = true;
        pilhaTipos.push(s->tipo);
        idAtribuicao = nome;
        break;
    }

    // ── #11: Marca identificador como inicializado (lado esq. de leia) ─
    case 11:
    {
        Simbolo* s = buscarSimbolo(idAtribuicao);
        if (s != nullptr) {
            s->inicializado = true;
            s->qtdUsos--;
            s->usada = (s->qtdUsos > 0);
        }
        if (!pilhaTipos.empty()) pilhaTipos.pop();
        
        string msgParaRemover = "AVISO: Variavel '" + idAtribuicao + "' usada sem inicializar (lixo de memoria).";
        for (auto it = avisos.rbegin(); it != avisos.rend(); ++it) {
            if (*it == msgParaRemover) {
                avisos.erase(std::next(it).base());
                break;
            }
        }
        break;
    }

    // ── #12: Empilha tipo do literal ─────────────────────────────────────────
    case 12:
    {
        ultimoValorLido = token->getLexeme();
        int tipo = SemanticTable::ERR;
        switch (token->getId()) {
            case t_INT_LITERAL:
            case t_BIN_LITERAL:
            case t_HEX_LITERAL:
                tipo = SemanticTable::INT;
                break;
            case t_REAL_LITERAL:
                tipo = SemanticTable::FLO;
                break;
            case t_CHAR_LITERAL:
                tipo = SemanticTable::CHA;
                break;
            case t_STRING_LITERAL:
                tipo = SemanticTable::STR;
                break;
            case t_BOOL_VERDADEIRO:
            case t_BOOL_FALSO:
                tipo = SemanticTable::BOO;
                break;
            default:
                tipo = SemanticTable::ERR;
                break;
        }
        pilhaTipos.push(tipo);
        break;
    }

    // ── #13: Operador relacional — calcula tipo resultante ───────────────────
    case 13:
    {
        if (pilhaTipos.size() < 2) break;
        int tipo2 = pilhaTipos.top(); pilhaTipos.pop();
        int tipo1 = pilhaTipos.top(); pilhaTipos.pop();
        int result = SemanticTable::resultType(tipo1, tipo2, SemanticTable::REL);
        if (result == SemanticTable::ERR) {
            throw SemanticError(
                "Tipos incompativeis em expressao relacional.",
                token->getPosition());
        }
        pilhaTipos.push(result);
        break;
    }

    // ── #14: Operador aditivo (+/-) — calcula tipo resultante ────────────────
    case 14:
    {
        if (pilhaTipos.size() < 2) break;
        int tipo2 = pilhaTipos.top(); pilhaTipos.pop();
        int tipo1 = pilhaTipos.top(); pilhaTipos.pop();
        int idOp = t_OP_SOMA;
        if (!pilhaOperadores.empty()) {
            idOp = pilhaOperadores.top(); pilhaOperadores.pop();
        }
        int op = (idOp == t_OP_SOMA) ? SemanticTable::SUM : SemanticTable::SUB;
        int result = SemanticTable::resultType(tipo1, tipo2, op);
        if (result == SemanticTable::ERR) {
            throw SemanticError(
                "Tipos incompativeis em expressao aditiva.",
                token->getPosition());
        }
        pilhaTipos.push(result);
        break;
    }

    // ── #15, #21, #22: Operador multiplicativo/lógico — calcula tipo resultante ─────────
    case 21:
    case 22:
    case 15:
    {
        if (pilhaTipos.size() < 2) break;
        int tipo2 = pilhaTipos.top(); pilhaTipos.pop();
        int tipo1 = pilhaTipos.top(); pilhaTipos.pop();

        int idOp = t_OP_MULT;
        if (!pilhaOperadores.empty()) {
            idOp = pilhaOperadores.top(); pilhaOperadores.pop();
        }
        
        int op;
        switch (idOp) {
            case t_OP_MULT: op = SemanticTable::MUL; break;
            case t_OP_DIV:  op = SemanticTable::DIV; break;
            case t_OP_MOD:  op = SemanticTable::MOD; break;
            case t_OP_AND:  op = SemanticTable::AND; break;
            case t_OP_OR:   op = SemanticTable::OR_; break;
            default:        op = SemanticTable::MUL; break;
        }

        int result = SemanticTable::resultType(tipo1, tipo2, op);
        if (result == SemanticTable::ERR) {
            throw SemanticError(
                "Tipos incompativeis em expressao.",
                token->getPosition());
        }
        pilhaTipos.push(result);
        break;
    }

    // ── #17: Verifica compatibilidade de atribuição ───────────────────────────
    case 17:
    {
        if (pilhaTipos.empty()) break;
        int tipoExpr = pilhaTipos.top(); pilhaTipos.pop();

        if (pilhaAtribuicoes.empty()) break;
        string alvo = pilhaAtribuicoes.top();
        pilhaAtribuicoes.pop();

        Simbolo* s = buscarSimbolo(alvo);
        if (s == nullptr) break;

        int tipoId = s->tipo;
        int result = SemanticTable::atribType(tipoId, tipoExpr);

        if (result == SemanticTable::ERR) {
            ostringstream msg;
            msg << "Atribuicao invalida: tipo incompativel para '"
                << alvo << "'.";
            throw SemanticError(msg.str(), token->getPosition());
        }
        if (result == SemanticTable::WAR) {
            ostringstream msg;
            msg << "AVISO: Possivel perda de precisao na atribuicao de '" << alvo << "'.";
            avisos.push_back(msg.str());
        }

        // Marca como inicializado
        s->inicializado = true;
        break;
    }

    // ── #24: Inicia contagem de argumentos para chamada de função ──────────────
    case 24:
    {
        pilhaContadorArgs.push(0);
        break;
    }

    // ── #23: Verifica tipo de um argumento passado ─────────────────────────────
    case 23:
    {
        if (pilhaTipos.empty()) break;
        int tipoArg = pilhaTipos.top();
        pilhaTipos.pop();

        if (pilhaNomes.empty() || pilhaContadorArgs.empty()) break;
        string nomeFunc = pilhaNomes.top();
        int numParam = pilhaContadorArgs.top();

        Simbolo* func = buscarSimbolo(nomeFunc);
        if (func == nullptr) {
             pilhaContadorArgs.top()++;
             break;
        }
        if (func->modalidade != MOD_FUNCAO) {
             ostringstream msg;
             msg << "Identificador '" << nomeFunc << "' nao e uma funcao.";
             throw SemanticError(msg.str(), token->getPosition());
        }

        Simbolo* param = nullptr;
        for (Simbolo& s : tabelaSimbolos) {
            if (s.escopo == nomeFunc && s.numeroParametro == numParam) {
                param = &s;
                break;
            }
        }

        if (param == nullptr) {
            ostringstream msg;
            msg << "Muitos argumentos para a funcao '" << nomeFunc << "'.";
            throw SemanticError(msg.str(), token->getPosition());
        }

        int result = SemanticTable::atribType(param->tipo, tipoArg);
        if (result == SemanticTable::ERR) {
            ostringstream msg;
            msg << "Tipo incompativel no parametro " << numParam
                << " da funcao '" << nomeFunc << "'.";
            throw SemanticError(msg.str(), token->getPosition());
        }

        // Codegen: passagem por cópia — ACC tem o valor do argumento
        emitir("STO    " + param->escopo + "_" + param->id);

        pilhaContadorArgs.top()++;
        break;
    }

    // ── #25: Finaliza chamada de função COM argumentos ─────────────────────────
    case 25:
    {
        if (pilhaNomes.empty() || pilhaContadorArgs.empty()) break;
        string nomeFunc = pilhaNomes.top();
        int numArgsPassados = pilhaContadorArgs.top();
        pilhaContadorArgs.pop();
        pilhaNomes.pop();

        Simbolo* func = buscarSimbolo(nomeFunc);
        if (func == nullptr) {
            ostringstream msg;
            msg << "Identificador '" << nomeFunc << "' nao declarado.";
            throw SemanticError(msg.str(), token->getPosition());
        }
        if (func->modalidade != MOD_FUNCAO) {
            ostringstream msg;
            msg << "Identificador '" << nomeFunc << "' nao e uma funcao.";
            throw SemanticError(msg.str(), token->getPosition());
        }

        bool temMaisParams = false;
        for (Simbolo& s : tabelaSimbolos) {
            if (s.escopo == nomeFunc && s.numeroParametro == numArgsPassados) {
                temMaisParams = true;
                break;
            }
        }

        if (temMaisParams) {
            ostringstream msg;
            msg << "Poucos argumentos para a funcao '" << nomeFunc << "'.";
            throw SemanticError(msg.str(), token->getPosition());
        }

        // Codegen: argumentos já copiados para as células de parâmetro
        emitir("CALL   _" + nomeFunc);
        ultimoAcessoChamada = true;

        pilhaTipos.push(func->tipo);
        func->qtdUsos++;
        func->usada = true;
        idAtribuicao = nomeFunc;
        break;
    }

    // ── #26: Finaliza chamada de função SEM argumentos ─────────────────────────
    case 26:
    {
        if (pilhaNomes.empty()) break;
        string nomeFunc = pilhaNomes.top();
        pilhaNomes.pop();

        Simbolo* func = buscarSimbolo(nomeFunc);
        if (func == nullptr) {
            ostringstream msg;
            msg << "Identificador '" << nomeFunc << "' nao declarado.";
            throw SemanticError(msg.str(), token->getPosition());
        }
        if (func->modalidade != MOD_FUNCAO) {
            ostringstream msg;
            msg << "Identificador '" << nomeFunc << "' nao e uma funcao.";
            throw SemanticError(msg.str(), token->getPosition());
        }

        bool esperaParams = false;
        for (Simbolo& s : tabelaSimbolos) {
            if (s.escopo == nomeFunc && s.numeroParametro == 0) {
                esperaParams = true;
                break;
            }
        }

        if (esperaParams) {
            ostringstream msg;
            msg << "Poucos argumentos para a funcao '" << nomeFunc << "'.";
            throw SemanticError(msg.str(), token->getPosition());
        }

        // Codegen: chamada sem argumentos
        emitir("CALL   _" + nomeFunc);
        ultimoAcessoChamada = true;

        pilhaTipos.push(func->tipo);
        func->qtdUsos++;
        func->usada = true;
        idAtribuicao = nomeFunc;
        break;
    }

    // ── #27: Valida se o tamanho do vetor na declaração é inteiro ────────────
    case 27:
    {
        if (pilhaTipos.empty()) break;
        int tipoTamanho = pilhaTipos.top();
        pilhaTipos.pop();
        if (tipoTamanho != SemanticTable::INT) {
            throw SemanticError("O tamanho do vetor deve ser um valor inteiro.", token->getPosition());
        }
        
        try {
            if (ultimoValorLido.size() > 2 && ultimoValorLido.substr(0, 2) == "0b") {
                tabelaSimbolos.back().tamanhoVetor = stoi(ultimoValorLido.substr(2), nullptr, 2);
            } else {
                tabelaSimbolos.back().tamanhoVetor = stoi(ultimoValorLido, nullptr, 0);
            }
        } catch(...) {
            tabelaSimbolos.back().tamanhoVetor = 0;
        }
        
        dentroDeclaracaoVetor = false;
        break;
    }

    // ══════════════════════════════════════════════════════════════════════════
    // ══  AÇÕES DE GERAÇÃO DE CÓDIGO BIP  ═════════════════════════════════════
    // ══════════════════════════════════════════════════════════════════════════

    // ── #54: Marca LHS da atribuição (após #16) ──────────────────────────────────
    case 54:
    {
        // pilhaAtribuicoes já tem o nome do alvo (empilhado por #16)
        if (!pilhaAtribuicoes.empty()) {
            nomeAlvoAtribuicao = pilhaAtribuicoes.top();
        }
        atribuicaoVetor = ultimoAcessoVetor;
        ultimoAcessoVetor = false;
        // Salva o índice do LHS em 1009 (a expressão RHS pode reusar 1002)
        if (atribuicaoVetor) {
            emitir("LD     1002");
            emitir("STO    1009");
        }
        break;
    }

    // ── #55: Conclui atribuição — emite STO ou STOV (após #17) ───────────────
    case 55:
    {
        if (atribuicaoVetor) {
            // ACC tem o valor da expressão (RHS)
            emitir("STO    1001");
            emitir("LD     1009"); // índice do LHS salvo em #54
            emitir("STO    $indr");
            emitir("LD     1001");
            emitir("STOV   " + resolveCelula(nomeAlvoAtribuicao));
        } else {
            emitir("STO    " + resolveCelula(nomeAlvoAtribuicao));
        }
        atribuicaoVetor = false;
        break;
    }

    // ── #56: Salva índice de vetor em 1002 ────────────────────────────────
    case 56:
    {
        emitir("STO    1002");
        ultimoAcessoVetor = true;
        break;
    }

    // ── #57: Leia — gera código de entrada (após #11) ─────────────────────
    case 57:
    {
        if (ultimoAcessoVetor) {
            // leia(vet[i])
            emitir("LD     $in_port");
            emitir("STO    1001");
            emitir("LD     1002");
            emitir("STO    $indr");
            emitir("LD     1001");
            emitir("STOV   " + resolveCelula(idAtribuicao));
        } else {
            // leia(x)
            emitir("LD     $in_port");
            emitir("STO    " + resolveCelula(idAtribuicao));
        }
        ultimoAcessoVetor = false;
        break;
    }

    // ── #58: Escreva — emite saída por argumento ──────────────────────────
    case 58:
    {
        emitir("STO    $out_port");
        break;
    }

    // ── #61: OP_SOMA reconhecido — salva 1º operando ──────────────────────
    case 61:
    {
        emitir("STO    1000");
        pilhaOperadoresCodegen.push(t_OP_SOMA);
        break;
    }

    // ── #62: Resultado da operação aritmética (após #14) ──────────────────
    case 62:
    {
        vector<string>& buf = bufAtual();
        int size = buf.size();
        if (size >= 2 && buf[size - 2] == "STO    1000") {
            string lastInstr = buf[size - 1];
            if (lastInstr.rfind("LDI    ", 0) == 0) {
                string val = lastInstr.substr(7);
                buf.pop_back();
                buf.pop_back();
                int op = t_OP_SOMA;
                if (!pilhaOperadoresCodegen.empty()) {
                    op = pilhaOperadoresCodegen.top();
                    pilhaOperadoresCodegen.pop();
                }
                if (op == t_OP_SUB) {
                    emitir("SUBI   " + val);
                } else {
                    emitir("ADDI   " + val);
                }
                break;
            }
            else if (lastInstr.rfind("LD     ", 0) == 0) {
                string varName = lastInstr.substr(7);
                buf.pop_back();
                buf.pop_back();
                int op = t_OP_SOMA;
                if (!pilhaOperadoresCodegen.empty()) {
                    op = pilhaOperadoresCodegen.top();
                    pilhaOperadoresCodegen.pop();
                }
                if (op == t_OP_SUB) {
                    emitir("SUB    " + varName);
                } else {
                    emitir("ADD    " + varName);
                }
                break;
            }
        }

        // Fallback padrão
        emitir("STO    1001");
        emitir("LD     1000");
        if (!pilhaOperadoresCodegen.empty()) {
            int op = pilhaOperadoresCodegen.top();
            pilhaOperadoresCodegen.pop();
            if (op == t_OP_SUB) {
                emitir("SUB    1001");
            } else {
                emitir("ADD    1001");
            }
        } else {
            emitir("ADD    1001");
        }
        break;
    }

    // ── #63: OP_SUB reconhecido — salva 1º operando ───────────────────────
    case 63:
    {
        emitir("STO    1000");
        pilhaOperadoresCodegen.push(t_OP_SUB);
        break;
    }

    // ── #70: Literal INT ou REAL — emite LDI ─────────────────────────────
    case 70:
    {
        if (!dentroDeclaracaoVetor) {
            emitir("LDI    " + ultimoValorLido);
        }
        break;
    }

    // ── #73: Acesso a variável/vetor como valor (em <fator>) ──────────────
    case 73:
    {
        if (ultimoAcessoChamada) {
            // Retorno de função: valor já está no ACC (pós-CALL)
            ultimoAcessoChamada = false;
            break;
        }
        vector<string>& buf = bufAtual();
        if (ultimoAcessoVetor) {
            string cel = resolveCelula(idAtribuicao);
            int size = buf.size();
            if (size >= 1 && buf[size - 1] == "STO    1002") {
                buf.pop_back(); // Remove STO 1002 (índice ainda no ACC)
                emitir("STO    $indr");
                emitir("LDV    " + cel);
            } else {
                emitir("LD     1002");
                emitir("STO    $indr");
                emitir("LDV    " + cel);
            }
            ultimoAcessoVetor = false;
        } else {
            // Otimização: elimina carga redundante (valor já no acumulador)
            string cel = resolveCelula(idAtribuicao);
            int size = buf.size();
            if (size >= 1 && buf[size - 1] == "STO    " + cel) {
                // O valor já está no Acumulador (ACC)
            } else {
                emitir("LD     " + cel);
            }
        }
        break;
    }

    // ══════════════════════════════════════════════════════════════════════════
    // ══  T5 / T6 — GERAÇÃO DE CÓDIGO ADICIONAL  ═════════════════════════════
    // ══════════════════════════════════════════════════════════════════════════

    // ── #33: retorne <expr>; — valor já no ACC ───────────────────────────────
    case 33:
        emitir("RETURN");
        break;

    // ── #41–#46: operador relacional — guarda operador e salva operando esq. ──
    case 41: operRelCg = "=="; emitir("STO    1003"); break;
    case 42: operRelCg = "!="; emitir("STO    1003"); break;
    case 43: operRelCg = ">";  emitir("STO    1003"); break;
    case 44: operRelCg = "<";  emitir("STO    1003"); break;
    case 45: operRelCg = ">="; emitir("STO    1003"); break;
    case 46: operRelCg = "<="; emitir("STO    1003"); break;

    // ── #81: emite a comparação relacional (resultado 0/1 no ACC) ─────────────
    case 81:
        emiteRelacionalCg();
        break;

    // ── #82/#83: OU lógico (||) ──────────────────────────────────────────────
    case 82: emitir("STO    1007"); break;
    case 83:
        emitir("STO    1008");
        emitir("LD     1007");
        emitir("OR     1008");
        break;

    // ── #84/#85: E lógico (&&) ───────────────────────────────────────────────
    case 84: emitir("STO    1005"); break;
    case 85:
        emitir("STO    1006");
        emitir("LD     1005");
        emitir("AND    1006");
        break;

    // ── #71/#72: literais booleanos ──────────────────────────────────────────
    case 71: emitir("LDI    1"); break;
    case 72: emitir("LDI    0"); break;

    // ── #74: NOT lógico ──────────────────────────────────────────────────────
    case 74:
    {
        string Lt = novoLabel("NOT");
        string Lf = novoLabel("NOT");
        emitir("ADDI   0");
        emitir("BEQ    " + Lt);
        emitir("LDI    0");
        emitir("JMP    " + Lf);
        emitir(Lt + ":");
        emitir("LDI    1");
        emitir(Lf + ":");
        break;
    }

    // ── #75: menos unário (0 - ACC) ──────────────────────────────────────────
    case 75:
        emitir("STO    1010");
        emitir("LDI    0");
        emitir("SUB    1010");
        break;

    // ── #90/#91/#93/#94: desvio condicional (se / se-senao) ──────────────────
    case 90:
    {
        string Lfalso = novoLabel("SEFALSO");
        emitir("ADDI   0");
        emitir("BEQ    " + Lfalso);
        pilhaCtrl.push_back({ Lfalso });
        break;
    }
    case 91:
    {
        string Lfalso = pilhaCtrl.back()[0];
        pilhaCtrl.pop_back();
        emitir(Lfalso + ":");
        break;
    }
    case 93:
    {
        string Lfalso = pilhaCtrl.back()[0];
        string Lfim   = novoLabel("SEFIM");
        emitir("JMP    " + Lfim);   // fim do "então" pula o "senao"
        emitir(Lfalso + ":");
        pilhaCtrl.back() = { Lfim };
        break;
    }
    case 94:
    {
        string Lfim = pilhaCtrl.back()[0];
        pilhaCtrl.pop_back();
        emitir(Lfim + ":");
        break;
    }

    // ── #95/#96/#97: enquanto (teste no início) ──────────────────────────────
    case 95:
    {
        string Lini = novoLabel("WHILE");
        string Lfim = novoLabel("WHEND");
        emitir(Lini + ":");
        pilhaCtrl.push_back({ Lini, Lfim });
        break;
    }
    case 96:
    {
        emitir("ADDI   0");
        emitir("BEQ    " + pilhaCtrl.back()[1]);
        break;
    }
    case 97:
    {
        vector<string> fr = pilhaCtrl.back();
        pilhaCtrl.pop_back();
        emitir("JMP    " + fr[0]);
        emitir(fr[1] + ":");
        break;
    }

    // ── #98/#99: faca … enquanto (teste no fim) ──────────────────────────────
    case 98:
    {
        string Lini = novoLabel("DO");
        emitir(Lini + ":");
        pilhaCtrl.push_back({ Lini });
        break;
    }
    case 99:
    {
        emitir("ADDI   0");
        emitir("BNE    " + pilhaCtrl.back()[0]);
        pilhaCtrl.pop_back();
        break;
    }

    // ── #100–#103: para (laço com variável de controle) ──────────────────────
    case 100:
    {
        string Lcond  = novoLabel("FORC");
        string Linc   = novoLabel("FORI");
        string Lcorpo = novoLabel("FORB");
        string Lfim   = novoLabel("FORF");
        emitir(Lcond + ":");
        pilhaCtrl.push_back({ Lcond, Linc, Lcorpo, Lfim });
        break;
    }
    case 101:
    {
        vector<string> fr = pilhaCtrl.back();
        emitir("ADDI   0");
        emitir("BEQ    " + fr[3]);   // condição falsa -> fim
        emitir("JMP    " + fr[2]);   // verdadeira -> corpo (pula incremento)
        emitir(fr[1] + ":");         // rótulo do incremento
        break;
    }
    case 102:
    {
        vector<string> fr = pilhaCtrl.back();
        emitir("JMP    " + fr[0]);   // volta à condição
        emitir(fr[2] + ":");         // rótulo do corpo
        break;
    }
    case 103:
    {
        vector<string> fr = pilhaCtrl.back();
        pilhaCtrl.pop_back();
        emitir("JMP    " + fr[1]);   // volta ao incremento
        emitir(fr[3] + ":");         // rótulo de fim
        break;
    }

    default:
        break;
    }
}

string Semantico::escopoAtual() const
{
    return pilhaEscopos.empty() ? "global" : pilhaEscopos.top();
}

void Semantico::inserirSimbolo(const string& id, Modalidade mod, int posicao)
{
    // Verifica se já existe
    string escopo = escopoAtual();
    for (const Simbolo& s : tabelaSimbolos) {
        if (s.id == id && s.escopo == escopo) {
            ostringstream msg;
            msg << "Identificador '" << id
                << "' ja declarado no escopo '" << escopo << "'.";
            throw SemanticError(msg.str(), posicao);
        }
    }

    Simbolo novo;
    novo.id           = id;
    novo.tipo         = tipoAtual;
    novo.modalidade   = mod;
    novo.escopo       = escopo;
    novo.inicializado = (mod == MOD_FUNCAO || mod == MOD_PARAMETRO || mod == MOD_PARAM_VETOR);
    novo.usada        = false;
    novo.qtdUsos      = 0;
    novo.posicao      = posicao;
    novo.numeroParametro = -1;
    novo.tamanhoVetor = 0;

    tabelaSimbolos.push_back(novo);
}

Simbolo* Semantico::buscarSimbolo(const string& id)
{
    stack<string> copia = pilhaEscopos;
    while (!copia.empty()) {
        string escopo = copia.top();
        copia.pop();
        for (Simbolo& s : tabelaSimbolos) {
            if (s.id == id && s.escopo == escopo)
                return &s;
        }
    }
    return nullptr;
}

Simbolo* Semantico::buscarSimboloEscopo(const string& id,
                                         const string& escopo)
{
    for (Simbolo& s : tabelaSimbolos) {
        if (s.id == id && s.escopo == escopo)
            return &s;
    }
    return nullptr;
}

int Semantico::tokenParaTipo(const Token* token)
{
    switch (token->getId()) {
        case t_TIPO_INTEIRO:  return SemanticTable::INT;
        case t_TIPO_REAL:     return SemanticTable::FLO;
        case t_TIPO_DOBRO:    return SemanticTable::FLO;
        case t_TIPO_CHAR:     return SemanticTable::CHA;
        case t_TIPO_STRING:   return SemanticTable::STR;
        case t_TIPO_BOOLEANO: return SemanticTable::BOO;
        case t_NULO:          return SemanticTable::ERR; 
        default:              return SemanticTable::ERR;
    }
}

int Semantico::tokenParaOperacao(const Token* token)
{
    switch (token->getId()) {
        case t_OP_SOMA:         return SemanticTable::SUM;
        case t_OP_SUB:          return SemanticTable::SUB;
        case t_OP_MULT:         return SemanticTable::MUL;
        case t_OP_DIV:          return SemanticTable::DIV;
        case t_OP_MOD:          return SemanticTable::MOD;
        case t_OP_IGUAL:
        case t_OP_DIFERENTE:
        case t_OP_MAIOR:
        case t_OP_MENOR:
        case t_OP_MAIOR_IGUAL:
        case t_OP_MENOR_IGUAL:  return SemanticTable::REL;
        case t_OP_AND:          return SemanticTable::AND;
        case t_OP_OR:           return SemanticTable::OR_;
        default:                return -1;
    }
}

void Semantico::verificarNaoUsadas()
{
    string escopo = escopoAtual();
    for (const Simbolo& s : tabelaSimbolos) {
        if (s.escopo == escopo && !s.usada && s.modalidade != MOD_FUNCAO) {
            ostringstream msg;
            msg << "AVISO: Identificador '" << s.id
                << "' declarado no escopo '" << escopo
                << "' nao foi utilizado.";
            avisos.push_back(msg.str());
        }
    }
}

string Semantico::nomeModalidade(Modalidade m) const
{
    switch (m) {
        case MOD_VARIAVEL:    return "variavel";
        case MOD_VETOR:       return "vetor";
        case MOD_PARAMETRO:   return "parametro";
        case MOD_PARAM_VETOR: return "parametro vetor";
        case MOD_FUNCAO:      return "funcao";
        default:              return "desconhecido";
    }
}

void Semantico::finalizarAnalise()
{
    // Verifica variáveis não utilizadas no escopo global ao final da compilação
    verificarNaoUsadas();
}

// ══════════════════════════════════════════════════════════════════════════════
// ══  MÉTODOS DE GERAÇÃO DE CÓDIGO BIP  ══════════════════════════════════════
// ══════════════════════════════════════════════════════════════════════════════

vector<string>& Semantico::bufAtual()
{
    return emFuncao ? codigoSubrotinas : codigoInstrucoes;
}

void Semantico::emitir(const string& instrucao)
{
    bufAtual().push_back(instrucao);
}

string Semantico::novoLabel(const string& prefixo)
{
    return prefixo + std::to_string(labelCount++);
}

// Resolve o nome de uma célula em .data: parâmetros usam "funcao_param".
string Semantico::resolveCelula(const string& nome)
{
    Simbolo* s = buscarSimbolo(nome);
    if (s != nullptr && (s->modalidade == MOD_PARAMETRO || s->modalidade == MOD_PARAM_VETOR))
        return s->escopo + "_" + s->id;
    return nome;
}

// Gera o resultado (0/1) de uma operação relacional usando o STATUS do BIP.
void Semantico::emiteRelacionalCg()
{
    emitir("STO    1004");   // operando direito
    emitir("LD     1003");   // operando esquerdo
    emitir("SUB    1004");   // ACC = esq - dir  (afeta Z e N)

    string Lv = novoLabel("REL");
    string Lf = novoLabel("REL");
    string b;
    if      (operRelCg == "==") b = "BEQ";
    else if (operRelCg == "!=") b = "BNE";
    else if (operRelCg == ">")  b = "BGT";
    else if (operRelCg == "<")  b = "BLT";
    else if (operRelCg == ">=") b = "BGE";
    else                        b = "BLE";

    emitir(b + "    " + Lv);
    emitir("LDI    0");
    emitir("JMP    " + Lf);
    emitir(Lv + ":");
    emitir("LDI    1");
    emitir(Lf + ":");
}

string Semantico::gerarSecaoData() const
{
    ostringstream data;
    data << ".data" << endl;
    for (const Simbolo& s : tabelaSimbolos) {
        if (s.modalidade == MOD_FUNCAO) continue;
        // Parâmetros usam célula qualificada "funcao_param" (evita colisão entre rotinas)
        bool ehParam = (s.modalidade == MOD_PARAMETRO || s.modalidade == MOD_PARAM_VETOR);
        string nomeCelula = ehParam ? (s.escopo + "_" + s.id) : s.id;
        if (s.modalidade == MOD_VETOR || s.modalidade == MOD_PARAM_VETOR) {
            data << nomeCelula << " : ";
            int n = s.tamanhoVetor > 0 ? s.tamanhoVetor : 1;
            for (int i = 0; i < n; i++) {
                if (i > 0) data << ", ";
                data << "0";
            }
            data << endl;
        } else {
            data << nomeCelula << " : 0" << endl;
        }
    }
    return data.str();
}

string Semantico::getCodigoGerado() const
{
    ostringstream saida;
    saida << gerarSecaoData() << endl;
    saida << ".text" << endl;
    saida << "_PRINCIPAL:" << endl;
    for (const string& instr : codigoInstrucoes) {
        saida << instr << endl;
    }
    saida << "HLT    0" << endl;
    // Sub-rotinas após o programa principal (alcançadas apenas via CALL)
    for (const string& instr : codigoSubrotinas) {
        saida << instr << endl;
    }
    return saida.str();
}
