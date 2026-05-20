#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "Token.h"
#include "SemanticError.h"
#include <string>

class Semantico
{
public:
    void executeAction(int action, const Token *token);

    // Retorna o código ASM completo — chamar após parse()
    std::string getCodigoASM() const;

private:
    // ---------- seções do ASM ----------
    std::string secaoData;
    std::string secaoText;

    void geraData (const std::string& nome, const std::string& valor);
    void geraInstr(const std::string& instr, const std::string& op = "");
    void geraLabel(const std::string& label);

    // ---------- controle de bloco ----------
    int blocoDepth = 0;

    // ---------- declarações ----------
    std::string nomeIdDecl;
    std::string tamanhoVetor = "0";
    bool        ehVetor      = false;

    // ---------- atribuição ----------
    std::string nomeIdAtrib;
    bool        atribEhVetor = false;

    // ---------- acesso / leitura ----------
    std::string nomeIdAcesso;
    bool        acessoEhVetor = false;

    // ---------- expressões ----------
    bool        flagOp     = false;
    bool        primeiroOp = true;
    std::string oper;
};

#endif
