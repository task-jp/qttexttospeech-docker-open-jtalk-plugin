TEMPLATE = lib
CONFIG += plugin
QT = texttospeech multimedia

PLUGIN_TYPE = texttospeech
PLUGIN_CLASS_NAME = Engine
load(qt_plugin)

SOURCES += \
    engine.cpp

HEADERS += \
    engine.h \
    plugin.h

DISTFILES += \
    plugin.json
