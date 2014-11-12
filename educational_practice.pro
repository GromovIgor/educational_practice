QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = educational_practice
TEMPLATE = app

SOURCES += main.cpp\
    compression.cpp \
    decompression.cpp \
    Dialog.cpp

HEADERS  += \
    compression.h \
    decompression.h \
    Dialog.h

FORMS    += \
    compression.ui \
    decompression.ui \
    Dialog.ui
