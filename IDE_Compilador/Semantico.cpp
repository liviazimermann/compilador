#include "Semantico.h"
#include "Constants.h"

// ============================================================
//  Helpers
// ============================================================

void Semantico::geraData(const std::string& nome, const std::string& valor) {
    secaoData += "    " + nome + " : " + valor + "\n";
}

void Semantico::geraInstr(const std::string& instr, const std::string& op) {
    if (op.empty())
        secaoText += "    " + instr + "\n";
    else
        secaoText += "    " + instr + "    " + op + "\n";
}

void Semantico::geraLabel(const std::string& label) {
    secaoText += label + ":\n";
}

std::string Semantico::getCodigoASM() const {
    return ".data\n" + secaoData + ".text\n" + secaoText;
}

// ============================================================
//  executeAction
// ============================================================

void Semantico::executeAction(int action, const Token *token)
{
    switch (action)
    {
    // --------------------------------------------------------
    // #52 — captura nome do identificador na declaração
    // --------------------------------------------------------
    case 52:
        nomeIdDecl   = token->getLexeme();
        ehVetor      = false;
        tamanhoVetor = "0";
        break;

    // --------------------------------------------------------
    // #53 — captura tamanho do vetor [N]
    // --------------------------------------------------------
    case 53:
        tamanhoVetor = token->getLexeme();
        ehVetor      = true;
        break;

    // --------------------------------------------------------
    // #51 — fecha declaração, emite no .data
    // --------------------------------------------------------
    case 51:
        if (ehVetor)
            geraData(nomeIdDecl, "0[" + tamanhoVetor + "]");
        else
            geraData(nomeIdDecl, "0");
        ehVetor = false;
        break;

    // --------------------------------------------------------
    // #50 — emite _PRINCIPAL:
    // --------------------------------------------------------
    case 50:
        geraLabel("_PRINCIPAL");
        break;

    // --------------------------------------------------------
    // #10 / #11 — controle de bloco
    // --------------------------------------------------------
    case 10:
        blocoDepth++;
        break;

    case 11:
        blocoDepth--;
        if (blocoDepth == 0)
            geraInstr("HLT", "0");
        break;

    // --------------------------------------------------------
    // #20 — captura IDENTIFICADOR em acesso_ou_chamada
    // --------------------------------------------------------
    case 20:
        nomeIdAcesso  = token->getLexeme();
        acessoEhVetor = false;
        break;

    // --------------------------------------------------------
    // #56 — fecha expr do índice em QUALQUER acesso_ou_chamada
    //        (lado esquerdo OU direito)
    //        Apenas seta flag e salva índice em 1002.
    //        Quem decide o que emitir é #54 (esq) ou #73 (dir).
    // --------------------------------------------------------
    case 56:
        geraInstr("STO", "1002");   // guarda índice calculado
        acessoEhVetor = true;
        break;

    // --------------------------------------------------------
    // #54 — lado esquerdo da atribuição
    //        Se for vetor: índice já está em 1002
    //        Se for variável: só salva o nome
    // --------------------------------------------------------
    case 54:
        nomeIdAtrib  = nomeIdAcesso;
        atribEhVetor = acessoEhVetor;
        primeiroOp   = true;
        flagOp       = false;
        break;

    // --------------------------------------------------------
    // #55 — fecha atribuição
    //   variável → STO nome
    //   vetor    → STO 1001 / LD 1002 / STO $indr / LD 1001 / STOV nome
    // --------------------------------------------------------
    case 55:
        if (!atribEhVetor) {
            geraInstr("STO", nomeIdAtrib);
        } else {
            geraInstr("STO",  "1001");
            geraInstr("LD",   "1002");
            geraInstr("STO",  "$indr");
            geraInstr("LD",   "1001");
            geraInstr("STOV", nomeIdAtrib);
        }
        atribEhVetor = false;
        flagOp       = false;
        primeiroOp   = true;
        break;

    // --------------------------------------------------------
    // #73 — acesso_ou_chamada usado como fator em expressão
    //        (lado direito)
    //        Se for vetor: índice em 1002 → STO $indr + LDV
    //        Se for variável simples: LD nome
    // --------------------------------------------------------
    case 73:
        if (acessoEhVetor) {
            geraInstr("LD",  "1002");
            geraInstr("STO", "$indr");
            geraInstr("LDV", nomeIdAcesso);
        } else {
            geraInstr("LD", nomeIdAcesso);
        }
        acessoEhVetor = false;
        primeiroOp    = false;
        break;

    // --------------------------------------------------------
    // #57 — leia(var) ou leia(vet[expr])
    // --------------------------------------------------------
    case 57:
        if (!acessoEhVetor) {
            geraInstr("LD",  "$in_port");
            geraInstr("STO", nomeIdAcesso);
        } else {
            geraInstr("LD",   "1002");
            geraInstr("STO",  "$indr");
            geraInstr("LD",   "$in_port");
            geraInstr("STOV", nomeIdAcesso);
        }
        acessoEhVetor = false;
        break;

    // --------------------------------------------------------
    // #58 — escreva: valor no ACC → $out_port
    // --------------------------------------------------------
    case 58:
        geraInstr("STO", "$out_port");
        break;

    // --------------------------------------------------------
    // Aritméticos: #61(+) #63(-) #62(emite)
    // --------------------------------------------------------
    case 61:
        oper   = "+";
        flagOp = true;
        if (!primeiroOp) geraInstr("STO", "1000");
        break;

    case 63:
        oper   = "-";
        flagOp = true;
        if (!primeiroOp) geraInstr("STO", "1000");
        break;

    case 62:
        if (flagOp) {
            geraInstr("STO", "1001");
            geraInstr("LD",  "1000");
            if (oper == "+") geraInstr("ADD", "1001");
            else             geraInstr("SUB", "1001");
            flagOp     = false;
            primeiroOp = false;
        }
        break;

    // --------------------------------------------------------
    // Bit a bit: #64(&) #66(|) #67(^) #68(<<) #69(>>) #65(emite)
    // --------------------------------------------------------
    case 64: oper = "&";  flagOp = true; if (!primeiroOp) geraInstr("STO","1000"); break;
    case 66: oper = "|";  flagOp = true; if (!primeiroOp) geraInstr("STO","1000"); break;
    case 67: oper = "^";  flagOp = true; if (!primeiroOp) geraInstr("STO","1000"); break;
    case 68: oper = "<<"; flagOp = true; if (!primeiroOp) geraInstr("STO","1000"); break;
    case 69: oper = ">>"; flagOp = true; if (!primeiroOp) geraInstr("STO","1000"); break;

    case 65:
        if (flagOp) {
            geraInstr("STO", "1001");
            geraInstr("LD",  "1000");
            if      (oper == "&")  geraInstr("AND", "1001");
            else if (oper == "|")  geraInstr("OR",  "1001");
            else if (oper == "^")  geraInstr("XOR", "1001");
            else if (oper == "<<") geraInstr("SLL", "1001");
            else if (oper == ">>") geraInstr("SRL", "1001");
            flagOp     = false;
            primeiroOp = false;
        }
        break;

    // --------------------------------------------------------
    // #70 — literal numérico → LDI valor
    // --------------------------------------------------------
    case 70:
        geraInstr("LDI", token->getLexeme());
        primeiroOp = false;
        break;

    // --------------------------------------------------------
    // #71 / #72 — verdadeiro / falso
    // --------------------------------------------------------
    case 71: geraInstr("LDI", "1"); primeiroOp = false; break;
    case 72: geraInstr("LDI", "0"); primeiroOp = false; break;

    default:
        break;
    }
}
