#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "Token.h"
#include "SemanticError.h"
#include "SemanticTable.h"

#include <string>
#include <vector>
#include <stack>

using namespace std;

// tipos
enum Modalidade {
    MOD_VARIAVEL,
    MOD_VETOR,
    MOD_PARAMETRO,
    MOD_PARAM_VETOR,
    MOD_FUNCAO
};

class Simbolo{
public:
    string      id;
    int         tipo;
    Modalidade  modalidade;
    string      escopo;
    bool        inicializado;
    bool        usada;
    int         qtdUsos;
    int         posicao;
    int         numeroParametro;
    int         tamanhoVetor;
};

class Semantico
{
public:
    Semantico();

    void executeAction(int action, const Token *token);

    const vector<Simbolo>& getTabelaSimbolos() const { return tabelaSimbolos; }
    const vector<string>& getAvisos() const { return avisos; }
    
    void finalizarAnalise();

private:
    // Tabela de símbolos
    vector<Simbolo> tabelaSimbolos;
    vector<string>  avisos;

    // escopos
    stack<string> pilhaEscopos;
    int contadorEscopo;

    // aux para declaração
    int    tipoAtual;
    string nomeAtualFuncao;
    int    tipoAtualFuncao;
    int    contadorParametros;
    string ultimoValorLido;

    // verifica tipos
    vector<int> variaveisSemTipo;
    vector<int> funcoesSemTipo;
    stack<int> pilhaTipos;
    string idAtribuicao;

    // tratamento de escopos
    stack<string> pilhaNomes;
    stack<string> pilhaAtribuicoes;
    stack<int> pilhaOperadores;
    stack<int> pilhaContadorArgs;

    string escopoAtual() const;
    void inserirSimbolo(const string& id, Modalidade mod, int posicao);
    Simbolo* buscarSimbolo(const string& id);
    Simbolo* buscarSimboloEscopo(const string& id, const string& escopo);
    int       tokenParaTipo(const Token* token);
    int       tokenParaOperacao(const Token* token);
    void      verificarNaoUsadas();
    string nomeModalidade(Modalidade m) const;
};

#endif
