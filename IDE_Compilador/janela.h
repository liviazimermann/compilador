#ifndef JANELA_H
#define JANELA_H

#include <QMainWindow>
#include <QTableWidget>
#include "Semantico.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class janela;
}
QT_END_NAMESPACE

class janela : public QMainWindow
{
    Q_OBJECT

public:
    explicit janela(QWidget *parent = nullptr);
    ~janela() override;

private slots:
    void on_Compilar_clicked();

private:
    Ui::janela *ui;

    void exibirTabelaSimbolos(const Semantico& sem);
};

#endif // JANELA_H
