#pragma once

#include <QObject>
#include <QVariantMap>
#include "song_model.h"
#include "audio_player.h"
#include "playback_filter_model.h"

class PlaybackController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(bool shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QVariantMap currentSong READ currentSong NOTIFY currentSongChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(SongModel* model READ model NOTIFY modelChanged)
    Q_PROPERTY(PlaybackFilterModel* filterModel READ filterModel NOTIFY filterModelChanged)
    Q_PROPERTY(int currentProxyIndex READ currentProxyIndex NOTIFY currentIndexChanged)

    public:
        explicit PlaybackController(QObject *parent = nullptr);

        Q_INVOKABLE void playSong(int index);
        Q_INVOKABLE void next();
        Q_INVOKABLE void previous();
        Q_INVOKABLE void pause();
        Q_INVOKABLE void resume();
        Q_INVOKABLE void restart();
        Q_INVOKABLE void setPosition(qint64 pos);

        bool playing() const;
        bool shuffle() const;
        void setShuffle(bool enabled);
        int currentIndex() const;
        QVariantMap currentSong() const;
        qint64 position() const;
        qint64 duration() const;

        SongModel* model() const;
        PlaybackFilterModel* filterModel() const;

        // SETTER
        void setModel(SongModel* model);
        void setPlayer(AudioPlayer* player);

        int currentProxyIndex() const;

    signals:
        void playingChanged();
        void shuffleChanged();
        void currentIndexChanged();
        void currentSongChanged();
        void positionChanged();
        void durationChanged();
        void modelChanged();
        void filterModelChanged();

    private:
        SongModel* mModel;
        AudioPlayer* mPlayer;
        PlaybackFilterModel* mFilterModel;
        bool mShuffle = false;
        int mCurrentIndex = -1;
};
