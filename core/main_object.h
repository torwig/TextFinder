#ifndef MAINOBJECT_H
#define MAINOBJECT_H

#include "core/task_executer.h"
#include "gui/main_window.h"

#include <QObject>

class MainObject : public QObject
{
    Q_OBJECT

public:
    explicit MainObject(QObject* parent = nullptr);

signals:

public slots:
    void onExit();

private:
    MainWindow* mWindow;
    TaskExecuter* mTaskExecuter;

};

#endif // MAINOBJECT_H
