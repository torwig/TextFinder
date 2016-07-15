#include <QApplication>

#include <gui/main_window.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    MainWindow mw;
    mw.show();
    int rc = app.exec();
    return rc;
}
