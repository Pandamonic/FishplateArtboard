QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



RC_ICON = app_icon.ico



SOURCES += \
    addshapecommand.cpp \
    artboardview.cpp \
    clearallcommand.cpp \
    deletemultipleshapescommand.cpp \
    deleteshapecommand.cpp \
    ellipseshape.cpp \
    eraserpathshape.cpp \
    freehandpathshape.cpp \
    lineshape.cpp \
    main.cpp \
    mainwindow.cpp \
    moveshapecommand.cpp \
    rectangleshape.cpp \
    starshape.cpp

HEADERS += \
    abstractcommand.h \
    abstractshape.h \
    addshapecommand.h \
    artboardview.h \
    clearallcommand.h \
    deletemultipleshapescommand.h \
    deleteshapecommand.h \
    ellipseshape.h \
    eraserpathshape.h \
    freehandpathshape.h \
    lineshape.h \
    mainwindow.h \
    moveshapecommand.h \
    rectangleshape.h \
    shared_types.h \
    starshape.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
