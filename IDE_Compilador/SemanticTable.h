#ifndef SEMANTIC_TABLE_H
#define SEMANTIC_TABLE_H

/*
 * Tabela de compatibilidade de tipos para o Analisador Semântico.
 * Baseado no template do Prof. Eduardo Alves da Silva - UNIVALI.
 */

class SemanticTable {
public:
    enum Types {
        INT = 0,  // inteiro
        FLO = 1,  // real / dobro
        CHA = 2,  // char
        STR = 3,  // string
        BOO = 4   // booleano
    };

    enum Operations {
        SUM = 0,  // +
        SUB = 1,  // -
        MUL = 2,  // *
        DIV = 3,  // /
        REL = 4,  // == != > < >= <=
        MOD = 5,  // %
        POT = 6,  // (reservado)
        ROO = 7,  // (reservado)
        AND = 8,  // &&
        OR_ = 9   // ||
    };

    enum Status {
        ERR = -1,
        WAR =  0,
        OK_ =  1
    };

    // Tabela de tipos resultantes para expressoes: expTable[TP1][TP2][OP]
    // 5 tipos x 5 tipos x 10 operacoes
    static int const expTable[5][5][10];

    // Tabela de compatibilidade de atribuicao: atribTable[ESQ][DIR]
    // 5 tipos x 5 tipos
    static int const atribTable[5][5];

    static int resultType(int TP1, int TP2, int OP) {
        if (TP1 < 0 || TP2 < 0 || OP < 0) return ERR;
        if (TP1 > 4 || TP2 > 4 || OP > 9)  return ERR;
        return expTable[TP1][TP2][OP];
    }

    static int atribType(int TP1, int TP2) {
        if (TP1 < 0 || TP2 < 0) return ERR;
        if (TP1 > 4 || TP2 > 4) return ERR;
        return atribTable[TP1][TP2];
    }
};

//
// expTable[TP1][TP2][OP]
// Ordem operacoes: SUM, SUB, MUL, DIV, REL, MOD, POT, ROO, AND, OR_
// Ordem tipos:     INT, FLO, CHA, STR, BOO
//
inline int const SemanticTable::expTable[5][5][10] = {

    // TP1 = INT
    {
        // TP2=INT   SUM  SUB  MUL  DIV  REL  MOD  POT  ROO  AND  OR_
        /* INT */ {  INT, INT, INT, FLO, BOO, INT, INT, FLO, ERR, ERR },
        // TP2=FLO
        /* FLO */ {  FLO, FLO, FLO, FLO, BOO, ERR, FLO, FLO, ERR, ERR },
        // TP2=CHA
        /* CHA */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        // TP2=STR
        /* STR */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        // TP2=BOO
        /* BOO */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR }
    },

    // TP1 = FLO
    {
        /* INT */ {  FLO, FLO, FLO, FLO, BOO, ERR, FLO, FLO, ERR, ERR },
        /* FLO */ {  FLO, FLO, FLO, FLO, BOO, ERR, FLO, FLO, ERR, ERR },
        /* CHA */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* STR */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* BOO */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR }
    },

    // TP1 = CHA
    {
        /* INT */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* FLO */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* CHA */ {  STR, ERR, ERR, ERR, BOO, ERR, ERR, ERR, ERR, ERR },
        /* STR */ {  STR, ERR, ERR, ERR, BOO, ERR, ERR, ERR, ERR, ERR },
        /* BOO */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR }
    },

    // TP1 = STR
    {
        /* INT */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* FLO */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* CHA */ {  STR, ERR, ERR, ERR, BOO, ERR, ERR, ERR, ERR, ERR },
        /* STR */ {  STR, ERR, ERR, ERR, BOO, ERR, ERR, ERR, ERR, ERR },
        /* BOO */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR }
    },

    // TP1 = BOO
    {
        /* INT */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* FLO */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* CHA */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* STR */ {  ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR },
        /* BOO */ {  ERR, ERR, ERR, ERR, BOO, ERR, ERR, ERR, BOO, BOO }
    }
};

//
// atribTable[ESQ][DIR]
// Ordem: INT, FLO, CHA, STR, BOO
//
inline int const SemanticTable::atribTable[5][5] = {
    //         INT   FLO   CHA   STR   BOO
    /* INT */ { OK_, WAR,  ERR,  ERR,  ERR },
    /* FLO */ { WAR, OK_,  ERR,  ERR,  ERR },
    /* CHA */ { ERR, ERR,  OK_,  WAR,  ERR },
    /* STR */ { ERR, ERR,  OK_,  OK_,  ERR }, // string pode receber char
    /* BOO */ { ERR, ERR,  ERR,  ERR,  OK_ }
};

#endif // SEMANTIC_TABLE_H
