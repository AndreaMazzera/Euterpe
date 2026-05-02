#pragma once

#include <string>
#include <vector>
#include <utility>

class Song
{
    public:
        struct LyricLine {
            long long timeMs;
            std::string text;
        };

        explicit Song(std::string filePath, std::string contentUri = "");

        // -------------------------------------
        // Getters
        // -------------------------------------
        const std::string& getFilePath() const;
        const std::string& getContentUri() const;
        const std::string& getTitle() const;
        const std::string& getArtist() const;
        const std::string& getAlbum() const;
        const std::vector<LyricLine>& getLyrics() const;
        long long getDurationMs() const;
        const std::string& getSongCover() const;
        const std::string& getSortName() const;

        // -------------------------------------
        // Setters
        // -------------------------------------
        void setContentUri(std::string uri);
        void setTitle(std::string title);
        void setArtist(std::string artist);
        void setAlbum(std::string album);
        void setLyrics(std::vector<LyricLine> lyrics);
        void setDuration(long long duration);
        void setSongCover(std::string cover);

        // -------------------------------------
        // Utilities
        // -------------------------------------
        bool hasLyrics() const;

    private:
        void updateSortKey();

        std::string mFilePath;
        std::string mContentUri;
        std::string mTitle;
        std::string mArtist;
        std::string mAlbum;
        std::string mSongCover;
        std::string mSortKey;
        std::vector<LyricLine> mLyrics;
        long long mDurationMs = 0;
};
