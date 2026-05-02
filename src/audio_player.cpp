#include "include/audio_player.h"
#include <QDebug>

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent)
{
    mPlayer = new QMediaPlayer(this);
    mAudioOutput = new QAudioOutput(this);

    mPlayer->setAudioOutput(mAudioOutput);

    connect(mPlayer, &QMediaPlayer::positionChanged, this, &AudioPlayer::positionChanged);
    connect(mPlayer, &QMediaPlayer::durationChanged, this, &AudioPlayer::durationChanged);

    connect(mPlayer, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        emit playbackStateChanged();
        emit playingChanged();
    });

    connect(mPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            emit playbackFinished();
        }
    });

    connect(mPlayer, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString) {
        qWarning() << "AudioPlayer Error [" << error << "]: " << errorString;
        emit errorOccurred(errorString);
    });
}

void AudioPlayer::play(const QString &sourceStr)
{
    if (sourceStr.isEmpty()) return;

    QUrl url = sourceStr.startsWith("content://") ? QUrl(sourceStr) : QUrl::fromLocalFile(sourceStr);

    if (mPlayer->source() != url) {
        mPlayer->setSource(url);
    }

    mPlayer->play();
}

void AudioPlayer::pause()
{
    mPlayer->pause();
}

void AudioPlayer::resume()
{
    mPlayer->play();
}

void AudioPlayer::stop()
{
    mPlayer->stop();
}

void AudioPlayer::setPosition(qint64 pos)
{
    if (mPlayer->position() != pos)
    {
        mPlayer->setPosition(pos);
    }
}

bool AudioPlayer::isPlaying() const
{
    return mPlayer->playbackState() == QMediaPlayer::PlayingState;
}

qint64 AudioPlayer::position() const
{
    return mPlayer->position();
}

qint64 AudioPlayer::duration() const
{
    return mPlayer->duration();
}

QUrl AudioPlayer::source() const
{
    return mPlayer->source();
}

void AudioPlayer::setSource(const QUrl &url)
{
    if (mPlayer->source() != url)
    {
        mPlayer->setSource(url);
        emit sourceChanged();
    }
}

QMediaPlayer::PlaybackState AudioPlayer::playbackState() const
{
    return mPlayer->playbackState();
}
