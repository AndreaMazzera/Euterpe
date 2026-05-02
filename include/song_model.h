#pragma once

#include <vector>
#include <QAbstractListModel>
#include "song.h"

#include <QJniObject>
#include <QJniEnvironment>
#include <QDebug>
#include <QStringList>

class QFileInfo;

class SongModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY songCountChanged)

    public:
        enum Roles {
            TitleRole = Qt::UserRole + 1,
            ArtistRole,
            AlbumRole,
            DurationRole,
            FilePathRole,
            UriRole,
            CoverRole,
            LyricsRole,
            SortRole,
        };
        Q_ENUM(Roles)

        explicit SongModel(QObject *parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role) const override;
        QVariantMap get(int index) const;
        QHash<int, QByteArray> roleNames() const override;

        Q_INVOKABLE void clear();
        Q_INVOKABLE void scanMusicFolder();
        Q_INVOKABLE void updateSongMetadataAt(int row);
        Q_INVOKABLE void deleteSong(const QString &filePath);

        void requestAndroidPermissions();

    public slots:
        void updateFullMetadata(const QString &path, const QString &t, const QString &ar, const QString &al, const QString &ly, const QString &cover);

    signals:
        void songCountChanged();
        void permissionsGranted();
        void permissionsDenied();

    protected:
        std::vector<Song::LyricLine> parseRawLyrics(const std::string& raw);

    private:
        std::vector<Song> mSongList;
        QJniObject mJavaWatcher;
};
