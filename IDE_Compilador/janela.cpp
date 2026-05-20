#include "janela.h"
#include "./ui_janela.h"
#include "Lexico.h"
#include "Sintatico.h"
#include "Semantico.h"

#include <iostream>
using namespace std;

janela::janela(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::janela)
{
    ui->setupUi(this);
}

janela::~janela()
{
    delete ui;
}

void janela::on_Compilar_clicked()
{
    Lexico    Lex;
    Sintatico Sint;
    Semantico Sem;

    QString codigoQt = ui->inputCodigo->toPlainText();
    string  codigoC  = codigoQt.toStdString();

    Lex.setInput(codigoC.c_str());

    // Limpa saídas anteriores
    ui->outputErro->clear();
    ui->outputASM->clear();

    try {
        Sint.parse(&Lex, &Sem);
        ui->outputErro->setPlainText("Compilado com sucesso!");

        // Exibe o código Assembly gerado
        QString asmCode = QString::fromStdString(Sem.getCodigoASM());
        ui->outputASM->setPlainText(asmCode);

    } catch (LexicalError err) {
        ui->outputErro->setPlainText(
            "Problema lexico: " + QString::fromStdString(err.getMessage()));
    } catch (SyntacticError err) {
        ui->outputErro->setPlainText(
            "Problema sintatico: " + QString::fromStdString(err.getMessage()));
    } catch (SemanticError err) {
        ui->outputErro->setPlainText(
            "Problema semantico: " + QString::fromStdString(err.getMessage()));
    }
}
