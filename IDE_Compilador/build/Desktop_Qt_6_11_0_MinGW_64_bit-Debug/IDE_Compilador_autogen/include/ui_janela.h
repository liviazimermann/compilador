/********************************************************************************
** Form generated from reading UI file 'janela.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_JANELA_H
#define UI_JANELA_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_janela
{
public:
    QWidget *centralwidget;
    QTextEdit *inputCodigo;
    QTextEdit *outputErro;
    QLabel *labelTabela;
    QTableWidget *tabelaSimbolos;
    QPushButton *Compilar;

    void setupUi(QMainWindow *janela)
    {
        if (janela->objectName().isEmpty())
            janela->setObjectName("janela");
        janela->resize(1919, 827);
        centralwidget = new QWidget(janela);
        centralwidget->setObjectName("centralwidget");
        inputCodigo = new QTextEdit(centralwidget);
        inputCodigo->setObjectName("inputCodigo");
        inputCodigo->setGeometry(QRect(10, 10, 1080, 551));
        QFont font;
        font.setPointSize(13);
        inputCodigo->setFont(font);
        outputErro = new QTextEdit(centralwidget);
        outputErro->setObjectName("outputErro");
        outputErro->setEnabled(true);
        outputErro->setGeometry(QRect(10, 570, 1081, 251));
        QFont font1;
        font1.setPointSize(12);
        outputErro->setFont(font1);
        outputErro->setReadOnly(true);
        labelTabela = new QLabel(centralwidget);
        labelTabela->setObjectName("labelTabela");
        labelTabela->setGeometry(QRect(1100, 10, 200, 20));
        QFont font2;
        font2.setPointSize(11);
        font2.setBold(true);
        labelTabela->setFont(font2);
        tabelaSimbolos = new QTableWidget(centralwidget);
        if (tabelaSimbolos->columnCount() < 8)
            tabelaSimbolos->setColumnCount(8);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tabelaSimbolos->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tabelaSimbolos->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tabelaSimbolos->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tabelaSimbolos->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tabelaSimbolos->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tabelaSimbolos->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tabelaSimbolos->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tabelaSimbolos->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        tabelaSimbolos->setObjectName("tabelaSimbolos");
        tabelaSimbolos->setGeometry(QRect(1100, 30, 811, 751));
        QFont font3;
        font3.setPointSize(11);
        tabelaSimbolos->setFont(font3);
        tabelaSimbolos->setAlternatingRowColors(true);
        tabelaSimbolos->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
        tabelaSimbolos->setWordWrap(true);
        Compilar = new QPushButton(centralwidget);
        Compilar->setObjectName("Compilar");
        Compilar->setGeometry(QRect(1810, 790, 100, 30));
        Compilar->setFont(font3);
        janela->setCentralWidget(centralwidget);

        retranslateUi(janela);

        QMetaObject::connectSlotsByName(janela);
    } // setupUi

    void retranslateUi(QMainWindow *janela)
    {
        janela->setWindowTitle(QCoreApplication::translate("janela", "Compilador - IDE", nullptr));
        inputCodigo->setPlaceholderText(QCoreApplication::translate("janela", "Digite o c\303\263digo aqui...", nullptr));
        outputErro->setPlaceholderText(QCoreApplication::translate("janela", "Mensagens do compilador...", nullptr));
        labelTabela->setText(QCoreApplication::translate("janela", "Tabela de S\303\255mbolos:", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tabelaSimbolos->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("janela", "ID", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tabelaSimbolos->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("janela", "Tipo", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tabelaSimbolos->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("janela", "Escopo", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tabelaSimbolos->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("janela", "Modalidade", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tabelaSimbolos->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("janela", "Inicializado", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tabelaSimbolos->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("janela", "Usada", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tabelaSimbolos->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("janela", "Num. Param", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tabelaSimbolos->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("janela", "Tamanho", nullptr));
        Compilar->setText(QCoreApplication::translate("janela", "Compilar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class janela: public Ui_janela {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_JANELA_H
