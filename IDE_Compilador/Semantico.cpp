#include "Semantico.h"
#include "Constants.h"

#include <sstream>
#include <iostream>

using namespace std;

Semantico::Semantico()
    : tipoAtual(-1), tipoAtualFuncao(-1), contadorEscopo(0), contadorParametros(0)
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
        break;
    }

    // ── #5: Fecha escopo da função ───────────────────────────────────────────
    case 5:
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
