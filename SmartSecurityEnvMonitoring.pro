#-------------------------------------------------
#
# Project created by QtCreator 2025-11-21T23:11:36
#
#-------------------------------------------------

QT       += core gui multimedia charts
QT += multimedia multimediawidgets
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SmartSecurityEnvMonitoring
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
    view/myview.cpp \
    data/singletondb.cpp \
    thread/camerathread.cpp \
    thread/encodetoh264.cpp \
    thread/playthread.cpp \
    thread/trancecode.cpp \
    view/myitem.cpp \
    view/mainwindow.cpp \
    view/settingwidget.cpp \
    data/device.cpp \
    model/devicemodel.cpp \
    Control/devicecontrol.cpp \
    data/alarminfo.cpp \
    model/usermodel.cpp \
    Control/usercontroller.cpp \
    data/captcha.cpp \
    view/loginwidget.cpp \
    view/registerwidget.cpp \
    view/videoplaybackwidget.cpp

HEADERS += \
        myview.h \
    view/myview.h \
    data/singletondb.h \
    thread/camerathread.h \
    thread/encodetoh264.h \
    thread/playthread.h \
    thread/trancecode.h \
    view/myitem.h \
    view/myview.h \
    view/mainwindow.h \
    view/settingwidget.h \
    data/device.h \
    model/devicemodel.h \
    Control/devicecontrol.h \
    data/alarminfo.h \
    model/usermodel.h \
    Control/usercontroller.h \
    data/captcha.h \
    view/loginwidget.h \
    view/registerwidget.h \
    view/videoplaybackwidget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DESTDIR = bin #目标文件exe生产的目录 bin dll
#$$PWD 当前工程的绝对路径
LIBS += $$PWD/lib/sqlite3.lib

INCLUDEPATH += $$PWD/ffmpeglib/include
LIBS += $$PWD/ffmpeglib/lib/avcodec.lib  \
        $$PWD/ffmpeglib/lib/avdevice.lib  \
        $$PWD/ffmpeglib/lib/avfilter.lib  \
        $$PWD/ffmpeglib/lib/avformat.lib  \
        $$PWD/ffmpeglib/lib/avutil.lib  \
        $$PWD/ffmpeglib/lib/swresample.lib  \
        $$PWD/ffmpeglib/lib/swscale.lib

RESOURCES += \
    resource.qrc




