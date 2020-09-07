#include "engine.h"

#include <QtCore/QBuffer>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtMultimedia/QAudioOutput>

namespace  {

struct RiffChunk {
    char id[4];
    qint32 size;
};

struct RiffHeader : public RiffChunk {
    char type[4];
};

struct WavFormat : public RiffChunk {
    qint16 audioFormat;
    qint16 numChannels;
    qint32 sampleRate;
    qint32 byteRate;
    qint16 blockAlign;
    qint16 bitsPerSample;
};

struct WavData : public RiffChunk {
};

struct WavHeader {
    RiffHeader riffHeader;
    WavFormat wavFormat;
    WavData wavData;
};

}

class Engine::Private
{
public:
    QAudioOutput *player = nullptr;
    QProcess *docker = nullptr;
    double pitch = 1.0;
    double rate = 1.0;
    double volume = 1.0;
    QLocale locale = QLocale("ja_JP.UTF-8");
    QVoice voice = QTextToSpeechEngine::createVoice("nitech_jp_atr503_m001", QVoice::Male, QVoice::Adult, QVariant());
};

Engine::Engine(QObject *parent)
    : QTextToSpeechEngine(parent)
    , d(new Private)
{}

Engine::~Engine() = default;

QVector<QLocale> Engine::availableLocales() const
{
    return { d->locale };
}

QVector<QVoice> Engine::availableVoices() const
{
    return { d->voice };
}

void Engine::say(const QString &text)
{
    auto docker = new QProcess(this);
    docker->setProgram("docker");
    docker->setArguments(QStringLiteral("run -i --rm u6kapps/open_jtalk").split(QLatin1Char(' ')));
    docker->start();
    connect(docker, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (d->player) {
            delete d->player;
            d->player = nullptr;
        }
        auto docker = qobject_cast<QProcess *>(sender());
        docker->setReadChannel(QProcess::StandardOutput);

        WavHeader header;
        docker->read(reinterpret_cast<char *>(&header), sizeof(header));

        QAudioFormat format;
        format.setSampleRate(header.wavFormat.sampleRate);
        format.setChannelCount(header.wavFormat.numChannels);
        format.setSampleSize(header.wavFormat.bitsPerSample);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::SignedInt);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(format)) {
            qWarning() << "Raw audio format not supported by backend, cannot play audio.";
            return;
        }

        d->player = new QAudioOutput(format, this);
        d->player->setVolume(d->volume);
        connect(d->player, &QAudioOutput::stateChanged, [this]() {
            emit stateChanged(state());
        });

        auto buffer = new QBuffer(d->player);
        auto data = docker->readAll();
        buffer->setData(data);
        buffer->open(QIODevice::ReadOnly);
        d->player->start(buffer);
        docker->deleteLater();
    });
    docker->waitForStarted();
    docker->write(text.toUtf8().replace("\n", " "));
    docker->closeWriteChannel();
}

void Engine::stop()
{
    if (d->player)
        d->player->stop();
}

void Engine::pause()
{
    if (d->player)
        d->player->suspend();
}

void Engine::resume()
{
    if (d->player)
        d->player->resume();
}

double Engine::rate() const
{
    return d->rate;
}

bool Engine::setRate(double rate)
{
    d->rate = rate;
    return true;
}

double Engine::pitch() const
{
    return d->pitch;
}

bool Engine::setPitch(double pitch)
{
    d->pitch = pitch;
    return true;
}

QLocale Engine::locale() const
{
    return d->locale;
}

bool Engine::setLocale(const QLocale &locale)
{
    return d->locale == locale;
}

double Engine::volume() const
{
    return d->volume;
}

bool Engine::setVolume(double volume)
{
    d->volume = volume;
    if (d->player)
        d->player->setVolume(volume);
    return true;
}

QVoice Engine::voice() const
{
    return d->voice;
}

bool Engine::setVoice(const QVoice &voice)
{
    return d->voice == voice;
}

QTextToSpeech::State Engine::state() const
{
    if (d->player) {
        switch (d->player->state()) {
        case QAudio::ActiveState:
            return QTextToSpeech::Speaking;
        case QAudio::SuspendedState:
            return QTextToSpeech::Paused;
        case QAudio::StoppedState:
        case QAudio::IdleState:
            return QTextToSpeech::Ready;
        case QAudio::InterruptedState:
        default:
            break;
        }
    } else {
        return QTextToSpeech::Ready;
    }
    return QTextToSpeech::BackendError;
}
