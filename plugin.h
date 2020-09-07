#ifndef PLUGIN_H
#define PLUGIN_H

#include <QtTextToSpeech/QTextToSpeechPlugin>
#include "engine.h"

class Plugin : public QObject, public QTextToSpeechPlugin
{
    Q_OBJECT
    Q_INTERFACES(QTextToSpeechPlugin)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.speech.tts.plugin/5.0" FILE "plugin.json")

public:
    explicit Plugin(QObject *parent = nullptr)
        : QObject(parent)
    {}

    QTextToSpeechEngine *createTextToSpeechEngine(
            const QVariantMap &parameters,
            QObject *parent,
            QString *errorString) const override
    {
        Q_UNUSED(parameters);
        Q_UNUSED(errorString);
        return new Engine(parent);
    }
};

#endif // PLUGIN_H
