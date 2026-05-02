#include "include/playback_controller.h"
#include <QRandomGenerator>
#include <QDebug>

PlaybackController::PlaybackController(QObject *parent) : QObject(parent),
    mModel(nullptr),
    mPlayer(nullptr),
    mFilterModel(nullptr),
    mShuffle(false),
    mCurrentIndex(-1) {
}

PlaybackFilterModel* PlaybackController::filterModel() const
{
    return mFilterModel;
}

void PlaybackController::setModel(SongModel* model)
{
    if (mModel == model) return;
    mModel = model;

    if (!mFilterModel) {
        mFilterModel = new PlaybackFilterModel(this);
    }
    mFilterModel->setSourceModel(mModel);

    emit modelChanged();
    emit filterModelChanged();
}

void PlaybackController::setPlayer(AudioPlayer *player)
{
    if (mPlayer == player) return;
    mPlayer = player;

    if (!mPlayer) return;

    connect(mPlayer, &AudioPlayer::positionChanged, this, &PlaybackController::positionChanged);
    connect(mPlayer, &AudioPlayer::durationChanged, this, &PlaybackController::durationChanged);
    connect(mPlayer, &AudioPlayer::playingChanged, this, &PlaybackController::playingChanged);
    connect(mPlayer, &AudioPlayer::playbackFinished, this, &PlaybackController::next);
}

void PlaybackController::playSong(int proxyIndex)
{
    if (!mFilterModel || !mPlayer || !mModel) return;

    if (proxyIndex < 0 || proxyIndex >= mFilterModel->rowCount()) return;

    QModelIndex proxyIdx = mFilterModel->index(proxyIndex, 0);
    QModelIndex sourceIdx = mFilterModel->mapToSource(proxyIdx);

    if (!sourceIdx.isValid()) return;

    mCurrentIndex = sourceIdx.row();

    emit currentIndexChanged();
    emit currentSongChanged();

    QString filePath = mModel->data(sourceIdx, SongModel::FilePathRole).toString();
    mPlayer->play(filePath);
}

void PlaybackController::next()
{
    // Safety check: if no songs are visible in the current filter, do nothing
    if (!mFilterModel || mFilterModel->rowCount() == 0)
        return;

    int currentProxyRow = -1;
    if (mCurrentIndex != -1)
    {
        QModelIndex sourceIdx = mModel->index(mCurrentIndex, 0);
        QModelIndex proxyIdx = mFilterModel->mapFromSource(sourceIdx);
        if (proxyIdx.isValid())
        {
            currentProxyRow = proxyIdx.row();
        }
    }

    int nextProxyRow;
    int count = mFilterModel->rowCount();

    if (mShuffle && count > 1)
    {
        do {
            nextProxyRow = QRandomGenerator::global()->bounded(count);
        } while (nextProxyRow == currentProxyRow);
    }
    else
    {
        // If currentProxyRow was -1 (song not in filter), this correctly starts from index 0
        nextProxyRow = (currentProxyRow + 1) % count;
    }

    playSong(nextProxyRow);
}

void PlaybackController::previous()
{
    if (!mFilterModel || mFilterModel->rowCount() == 0)
        return;

    int currentProxyRow = -1;
    if (mCurrentIndex != -1)
    {
        QModelIndex sourceIdx = mModel->index(mCurrentIndex, 0);
        currentProxyRow = mFilterModel->mapFromSource(sourceIdx).row();
    }

    if (mShuffle)
    {
        next();
        return;
    }

    int prevProxyRow = currentProxyRow - 1;
    if (prevProxyRow < 0)
        prevProxyRow = mFilterModel->rowCount() - 1;

    playSong(prevProxyRow);
}

void PlaybackController::pause()
{
    if (mPlayer)
        mPlayer->pause();
}

void PlaybackController::resume()
{
    if (mPlayer)
        mPlayer->resume();
}

void PlaybackController::restart()
{
    if (mPlayer)
        mPlayer->setPosition(0);
}

void PlaybackController::setPosition(qint64 pos)
{
    if (mPlayer)
        mPlayer->setPosition(pos);
}

bool PlaybackController::playing() const
{
    return mPlayer ? mPlayer->isPlaying() : false;
}

bool PlaybackController::shuffle() const
{
    return mShuffle;
}

void PlaybackController::setShuffle(bool enabled)
{
    if (mShuffle == enabled)
        return;

    mShuffle = enabled;
    emit shuffleChanged();
}

int PlaybackController::currentIndex() const
{
    return mCurrentIndex;
}

QVariantMap PlaybackController::currentSong() const
{
    if (!mModel || mCurrentIndex < 0 || mCurrentIndex >= mModel->rowCount())
        return {};

    return mModel->get(mCurrentIndex);
}

qint64 PlaybackController::position() const
{
    return mPlayer ? mPlayer->position() : 0;
}

qint64 PlaybackController::duration() const
{
    return mPlayer ? mPlayer->duration() : 0;
}

SongModel* PlaybackController::model() const
{
    return mModel;
}

int PlaybackController::currentProxyIndex() const
{
    if (!mFilterModel || mCurrentIndex == -1)
        return -1;

    QModelIndex sourceIdx = mModel->index(mCurrentIndex, 0);

    return mFilterModel->mapFromSource(sourceIdx).row();
}
