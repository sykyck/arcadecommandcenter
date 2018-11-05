#-------------------------------------------------
#
# Project created by QtCreator 2015-03-05T01:25:26
#
#-------------------------------------------------
include(qslog/QsLog.pri)

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = arcade-cmd-center
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qtsingleapplication/qtsinglecoreapplication.cpp \
    qtsingleapplication/qtsingleapplication.cpp \
    qtsingleapplication/qtlockedfile_unix.cpp \
    qtsingleapplication/qtlockedfile.cpp \
    qtsingleapplication/qtlocalpeer.cpp \
    global.cpp \
    updater.cpp \
    settings.cpp \
    databasemgr.cpp \
    encdec.cpp \
    qjson/serializerrunnable.cpp \
    qjson/serializer.cpp \
    qjson/qobjecthelper.cpp \
    qjson/parserrunnable.cpp \
    qjson/parser.cpp \
    qjson/json_scanner.cpp \
    qjson/json_scanner.cc \
    qjson/json_parser.cc \
    alertswidget.cpp \
    webservice.cpp \
    formpost/formpost.cpp \
    droneswidget.cpp \
    dronetaskswidget.cpp \
    locationswidget.cpp \
    softwarewidget.cpp \
    editdronetaskwidget.cpp \
    editsoftwarewidget.cpp \
    alertdetailwidget.cpp \
    editlocationwidget.cpp

HEADERS  += mainwindow.h \
    qtsingleapplication/qtsinglecoreapplication.h \
    qtsingleapplication/qtsingleapplication.h \
    qtsingleapplication/qtlockedfile.h \
    qtsingleapplication/qtlocalpeer.h \
    global.h \
    updater.h \
    settings.h \
    databasemgr.h \
    encdec.h \
    qjson/stack.hh \
    qjson/serializerrunnable.h \
    qjson/serializer.h \
    qjson/qobjecthelper.h \
    qjson/qjson_export.h \
    qjson/qjson_debug.h \
    qjson/position.hh \
    qjson/parserrunnable.h \
    qjson/parser_p.h \
    qjson/parser.h \
    qjson/location.hh \
    qjson/json_scanner.h \
    qjson/json_parser.hh \
    qjson/FlexLexer.h \
    alertswidget.h \
    alert.h \
    webservice.h \
    formpost/formpostinterface.h \
    formpost/formpost.h \
    droneswidget.h \
    drone.h \
    dronetaskswidget.h \
    dronetask.h \
    locationswidget.h \
    location.h \
    softwarewidget.h \
    software.h \
    editdronetaskwidget.h \
    taskrule.h \
    editsoftwarewidget.h \
    alertdetailwidget.h \
    editlocationwidget.h
