#include "janela.h"
#include <QApplication>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    janela w;
    w.show();
    return a.exec();
}
