#ifndef ENGINE_H
#define ENGINE_H

#include <QtTextToSpeech/QTextToSpeechEngine>

class Engine : public QTextToSpeechEngine
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = nullptr);
    ~Engine() override;

    QVector<QLocale> availableLocales() const override;
    QVector<QVoice> availableVoices() const override;

    void say(const QString &text) override;
    void stop() override;
    void pause() override;
    void resume() override;

    double rate() const override;
    bool setRate(double rate) override;
    double pitch() const override;
    bool setPitch(double pitch) override;
    QLocale locale() const override;
    bool setLocale(const QLocale &locale) override;
    double volume() const override;
    bool setVolume(double volume) override;
    QVoice voice() const override;
    bool setVoice(const QVoice &voice) override;
    QTextToSpeech::State state() const override;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // ENGINE_H
