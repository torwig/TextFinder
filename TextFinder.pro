QT += widgets
QT += network

CONFIG += c++11

MOC_DIR = moc/
OBJECTS_DIR = obj/
UI_DIR = ui/
DESTDIR = build/
RCC_DIR = rcc/

SOURCES += \
    main.cpp \
    gui/main_window.cpp \
    base/config.cpp \
    core/task_executer.cpp \
    core/page_downloader.cpp \
    base/text_finder.cpp

FORMS += \
    gui/main_window.ui

HEADERS += \
    gui/main_window.h \
    base/search_task.h \
    base/config.h \
    core/task_executer.h \
    core/page_downloader.h \
    base/text_finder.h
