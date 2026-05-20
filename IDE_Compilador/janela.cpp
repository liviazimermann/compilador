#include "janela.h"
#include "./ui_janela.h"
#include "Lexico.h"
#include "Sintatico.h"
#include "Semantico.h"

#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>

using namespace std;

janela::janela(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::janela)
{
    ui->setupUi(this);

    // Configura a tabela de símbolos
    ui->tabelaSimbolos->setColumnCount(8);
    QStringList cabecalhos;
    cabecalhos << "ID" << "Tipo" << "Escopo" << "Modalidade" << "Inicializado" << "Usada" << "Num. Param" << "Tamanho";
    ui->tabelaSimbolos->setHorizontalHeaderLabels(cabecalhos);
    ui->tabelaSimbolos->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tabelaSimbolos->horizontalHeader()->setStretchLastSection(true);
    ui->tabelaSimbolos->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tabelaSimbolos->setSelectionBehavior(QAbstractItemView::SelectRows);
}

janela::~janela()
{
    delete ui;
}

void janela::on_Compilar_clicked()
{
    Lexico   Lex;
    Sintatico Sint;
    Semantico Sem;

    QString codigoQt = ui->inputCodigo->toPlainText();
    string  codigoC  = codigoQt.toStdString();

    Lex.setInput(codigoC.c_str());

    // Limpa a tabela antes de compilar
    ui->tabelaSimbolos->setRowCount(0);
    ui->outputErro->setPlainText("");

    try {
        Sint.parse(&Lex, &Sem);
        Sem.finalizarAnalise();
        
        QString outputText = "Compilado com sucesso!";
        const vector<string>& avisos = Sem.getAvisos();
        if (!avisos.empty()) {
            outputText += "\n\nAvisos Semanticos:\n";
            for (const string& aviso : avisos) {
                outputText += QString::fromStdString(aviso) + "\n";
            }
        }
        
        ui->outputErro->setPlainText(outputText);
        exibirTabelaSimbolos(Sem);
    }
    catch (LexicalError err) {
        QString msgErro = QString::fromStdString(err.getMessage());
        ui->outputErro->setPlainText("Erro lexico: " + msgErro);
    }
    catch (SyntacticError err) {
        QString msgErro = QString::fromStdString(err.getMessage());
        ui->outputErro->setPlainText("Erro sintatico: " + msgErro);
    }
    catch (SemanticError err) {
        QString msgErro = QString::fromStdString(err.getMessage());
        QString outputText = "Erro semantico: " + msgErro;
        
        ui->outputErro->setPlainText(outputText);
    }
}

void janela::exibirTabelaSimbolos(const Semantico& sem)
{
    const vector<Simbolo>& tabela = sem.getTabelaSimbolos();

    ui->tabelaSimbolos->setRowCount(static_cast<int>(tabela.size()));

    auto nomeModalidade = [](Modalidade m) -> QString {
        switch (m) {
            case MOD_VARIAVEL:    return "variavel";
            case MOD_VETOR:       return "vetor";
            case MOD_PARAMETRO:   return "parametro";
            case MOD_PARAM_VETOR: return "param vetor";
            case MOD_FUNCAO:      return "funcao";
            default:              return "?";
        }
    };

    auto nomeTipo = [](int t) -> QString {
        switch (t) {
            case SemanticTable::INT: return "inteiro";
            case SemanticTable::FLO: return "real";
            case SemanticTable::CHA: return "char";
            case SemanticTable::STR: return "string";
            case SemanticTable::BOO: return "booleano";
            default:                 return "nulo";
        }
    };

    for (int i = 0; i < static_cast<int>(tabela.size()); ++i) {
        const Simbolo& s = tabela[i];
        ui->tabelaSimbolos->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(s.id)));
        ui->tabelaSimbolos->setItem(i, 1, new QTableWidgetItem(nomeTipo(s.tipo)));
        ui->tabelaSimbolos->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(s.escopo)));
        ui->tabelaSimbolos->setItem(i, 3, new QTableWidgetItem(nomeModalidade(s.modalidade)));
        ui->tabelaSimbolos->setItem(i, 4, new QTableWidgetItem(s.inicializado ? "sim" : "nao"));
        ui->tabelaSimbolos->setItem(i, 5, new QTableWidgetItem(s.usada        ? "sim" : "nao"));
        
        QString paramStr = (s.numeroParametro != -1) ? QString::number(s.numeroParametro) : "-";
        ui->tabelaSimbolos->setItem(i, 6, new QTableWidgetItem(paramStr));
        
        QString tamStr = (s.modalidade == MOD_VETOR) ? QString::number(s.tamanhoVetor) : "-";
        ui->tabelaSimbolos->setItem(i, 7, new QTableWidgetItem(tamStr));
    }
    
    // Ajusta o tamanho das colunas para caber o conteúdo longo (como os nomes dos escopos aninhados)
    ui->tabelaSimbolos->resizeColumnsToContents();
}
