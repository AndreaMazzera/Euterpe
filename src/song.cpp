#include "include/song.h"

Song::Song(std::string path, std::string contentUri)
    : mFilePath(std::move(path)),
    mContentUri(std::move(contentUri)),
    mDurationMs(0)
{
}

// -------------------------------------
// Getters
// -------------------------------------

const std::string& Song::getFilePath() const
{
    return mFilePath;
}

const std::string& Song::getContentUri() const
{
    return mContentUri;
}

const std::string& Song::getTitle() const
{
    return mTitle;
}

const std::string& Song::getArtist() const
{
    return mArtist;
}

const std::string& Song::getAlbum() const
{
    return mAlbum;
}

long long Song::getDurationMs() const
{
    return mDurationMs;
}

const std::string& Song::getSongCover() const
{
    return mSongCover;
}

const std::vector<Song::LyricLine>& Song::getLyrics() const
{
    return mLyrics;
}

const std::string& Song::getSortName() const
{
    return mSortKey;
}

// -------------------------------------
// Setters
// -------------------------------------

void Song::setLyrics(std::vector<LyricLine> lyrics)
{
    mLyrics = std::move(lyrics);
}

void Song::setContentUri(std::string uri)
{
    mContentUri = std::move(uri);
}

void Song::setTitle(std::string value)
{
    if (!value.empty()) {
        mTitle = std::move(value);
        updateSortKey();
    }
}

void Song::setArtist(std::string value)
{
    if (!value.empty()) {
        mArtist = std::move(value);
        updateSortKey();
    }
}

void Song::setAlbum(std::string value)
{
    mAlbum = std::move(value);
}

void Song::setDuration(long long value)
{
    mDurationMs = value;
}

void Song::setSongCover(std::string value)
{
    mSongCover = std::move(value);
}

// -------------------------------------
// Utilities
// -------------------------------------

void Song::updateSortKey()
{
    mSortKey = mArtist + " - " + mTitle;
}

bool Song::hasLyrics() const
{
    return !mLyrics.empty();
}
