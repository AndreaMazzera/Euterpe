#pragma once

#include <optional>
#include <QObject>
#include <QtCore/private/qandroidextras_p.h>

class MetadataService : public QObject, public QAndroidActivityResultReceiver
{
    Q_OBJECT

    public:
        explicit MetadataService(QObject *parent = nullptr) : QObject(parent) {}

        // Setters
        Q_INVOKABLE void setInitialState(const QString& title, const QString& artist, const QString& album);;
        Q_INVOKABLE void setMetadataTitle(const QString&);
        Q_INVOKABLE void setMetadataArtist(const QString&);
        Q_INVOKABLE void setMetadataAlbum(const QString&);
        Q_INVOKABLE void pickCoverImage();
        Q_INVOKABLE void pickLyricsFile();

        // Utilities
        Q_INVOKABLE void updateMetadata(const QString& filePath, const QString& uri);
        Q_INVOKABLE void removeSongFromAndroidDatabase(const QString &filePath);

        void handleActivityResult(int requestCode, int resultCode, const QJniObject &data) override;

    signals:
        void metadataSaved(bool success,
                           const QString& filePath,   // Chiave univoca per trovare la Song
                           const QString& title,      // mTitle
                           const QString& artist,     // mArtist
                           const QString& album,      // mAlbum
                           const QString& lyrics,     // mLyrics (USLT)
                           const QString& songCover); // mSongCover (Percorso cache JPG)
        void lyricsUpdated(const QString& filePath, const QString& newLyrics);

    public slots:
        void clearPendingData();

    private:
        std::optional<QString> mPendingTitleData;
        std::optional<QString> mPendingArtistData;
        std::optional<QString> mPendingAlbumData;
        QByteArray mPendingCoverData;
        QString    mPendingLyricsText;
};
