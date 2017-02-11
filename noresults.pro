TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG -= c++11

QMAKE_CXXFLAGS += -std=c++1z

LIBS += -lpthread

SOURCES += main.cpp
