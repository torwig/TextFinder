#include "main_object.h"

MainObject::MainObject(QObject* parent)
    : QObject(parent),
      mWindow(new MainWindow(this)),
      mTaskExecuter(new TaskExecuter(this))
{

}
