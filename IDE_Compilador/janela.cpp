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
    Lexico Lex;
    Sintatico Sint;
    Semantico Sem;

    QString codigoQt = ui->inputCodigo->toPlainText();
    string codigoC = codigoQt.toStdString();

    Lex.setInput(codigoC.c_str());

    try {
        Sint.parse(&Lex,&Sem);
        ui->outputErro->setPlainText("Compilado com sucesso!");
    } catch(LexicalError err) {
        QString msgErro = QString::fromStdString(err.getMessage());
        ui->outputErro->setPlainText("Problema lexico: " + msgErro);
    } catch (SyntacticError err) {
        QString msgErro = QString::fromStdString(err.getMessage());
        ui->outputErro->setPlainText("Problema sintatico: " + msgErro);
    } catch (SemanticError err) {
        QString msgErro = QString::fromStdString(err.getMessage());
        ui->outputErro->setPlainText("Problema semantico: " + msgErro);
    }
}

