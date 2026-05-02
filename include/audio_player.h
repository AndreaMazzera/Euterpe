#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

class AudioPlayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QMediaPlayer::PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)

public:
    explicit AudioPlayer(QObject *parent = nullptr);

    Q_INVOKABLE void play(const QString &sourceStr);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setPosition(qint64 pos);

    QUrl source() const;
    void setSource(const QUrl &url);

    QMediaPlayer::PlaybackState playbackState() const;
    bool isPlaying() const;
    qint64 position() const;
    qint64 duration() const;

signals:
    void playingChanged();
    void positionChanged();
    void durationChanged();
    void sourceChanged();
    void playbackStateChanged();
    void playbackFinished();
    void errorOccurred(const QString &errorMsg);

private:
    QMediaPlayer *mPlayer;
    QAudioOutput *mAudioOutput;
};
