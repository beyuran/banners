#-------------------------------------------------
#
# Project created by QtCreator 2017-11-14T16:45:36
#
#-------------------------------------------------

QT       += core gui printsupport

# debug console
CONFIG   += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Banners
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    programwindow.cpp \
    mouselabel.cpp \
    objectdetector.cpp \
    parametersdialog.cpp \
    serializer.cpp \
    imageviewer.cpp \
    bannerdialog.cpp \
    programwindowvideo.cpp \
    programwindowimage.cpp

HEADERS  += mainwindow.h \
    programwindow.h \
    mouselabel.h \
    objectdetector.h \
    parametersdialog.h \
    serializer.h \
    imageviewer.h \
    bannerdialog.h \
    structures.h \
    programwindowvideo.h \
    programwindowimage.h

FORMS    += mainwindow.ui \
    programwindow.ui \
    parametersdialog.ui \
    bannerdialog.ui


QMAKE_CXXFLAGS += -std=gnu++11
QMAKE_LFLAGS_RELEASE += -Wl,--allow-multiple-definition

CONFIG += -static
QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc

Opencv_VERSION = 300

LIBS += -Wl,-Bstatic -Wl,-Bdynamic

LIBS    += C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/lib/libopencv_world$${Opencv_VERSION}.a \
           #C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/lib/libopencv_contrib_world$${Opencv_VERSION}.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/lib/libopencv_xfeatures2d$${Opencv_VERSION}.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/lib/libopencv_features2d$${Opencv_VERSION}.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/lib/libopencv_imgproc$${Opencv_VERSION}.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/3rdparty/lib/libIlmImf.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/3rdparty/lib/liblibjasper.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/3rdparty/lib/liblibjpeg.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/3rdparty/lib/liblibpng.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/3rdparty/lib/liblibtiff.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/3rdparty/lib/liblibwebp.a \
           C:/library/opencv-3.0.0/opencv/build/x86/mingw_nopy/3rdparty/lib/libzlib.a

LIBS    += C:/Qt/Qt5.1.0_static/lib/libvfw32.a

DEFINES += _WINDOWS

INCLUDEPATH += C:/library/opencv-3.0.0/opencv/build/include
