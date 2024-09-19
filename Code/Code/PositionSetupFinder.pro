TEMPLATE = app
TARGET = Posefi

QT = core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

FORMS += \
    mainwindow.ui

HEADERS += \
    Constants.h \
    finder.h \
    posefi.h \
    solution.h

SOURCES += \
    finder.cpp \
    main.cpp \
    posefi.cpp
