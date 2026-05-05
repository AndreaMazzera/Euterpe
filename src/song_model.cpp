#include "include/song_model.h"
#include <QDir>
#include <QStandardPaths>
#include <QUrl>
#include <QFileInfo>
#include <QImage>
#include <QRegularExpression>

#include <tag.h>
#include <fileref.h>
#include <mpegfile.h>
#include <id3v2/frames/attachedpictureframe.h>
#include <id3v2/id3v2tag.h>
#include <id3v2/id3v2frame.h>
#include <id3v2/frames/unsynchronizedlyricsframe.h>

#include <QtCore/qnativeinterface.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/QPermission>
#include <QtCore/private/qandroidextras_p.h>
#include <QtCore/QSysInfo>

static SongModel* s_instance = nullptr;

SongModel::SongModel(QObject *parent) : QAbstractListModel(parent) { s_instance = this; }

int SongModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(mSongList.size());
}

QVariant SongModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mSongList.size())
        return {};

    const Song &song = mSongList[index.row()];

    switch (role)
    {
    case TitleRole:
        return QString::fromStdString(song.getTitle());
    case ArtistRole:
        return QString::fromStdString(song.getArtist());
    case AlbumRole:
        return QString::fromStdString(song.getAlbum());
    case DurationRole:
        return static_cast<qlonglong>(song.getDurationMs());
    case FilePathRole:
        return QString::fromStdString(song.getFilePath());
    case UriRole:
        return QString::fromStdString(song.getContentUri());
    case CoverRole:
        return QString::fromStdString(song.getSongCover());
    case SortRole:
        return QString::fromStdString(song.getSortName());
    case LyricsRole:
    {
        QVariantList list;
        for (const auto& line : song.getLyrics())
        {
            QVariantMap map;
            map["time"] = line.timeMs;
            map["text"] = QString::fromStdString(line.text);
            list.append(map);
        }
        return list;
    }
    }
    return {};
}

QVariantMap SongModel::get(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= static_cast<int>(mSongList.size()))
        return map;

    const Song &song = mSongList[index];
    map["title"]         = QString::fromStdString(song.getTitle());
    map["artist"]        = QString::fromStdString(song.getArtist());
    map["album"]         = QString::fromStdString(song.getAlbum());
    map["duration"]      = static_cast<qlonglong>(song.getDurationMs());
    map["filePath"]      = QString::fromStdString(song.getFilePath());
    map["uri"]           = QString::fromStdString(song.getContentUri());
    map["songCover"]     = QString::fromStdString(song.getSongCover());
    map["sortKey"]       = QString::fromStdString(song.getSortName());
    map["fileName"]      = QString::fromStdString(song.getFilePath());

    QVariantList lyricsList;
    for (const auto& line : song.getLyrics()) {
        QVariantMap lineMap;
        lineMap["time"] = line.timeMs;
        lineMap["text"] = QString::fromStdString(line.text);
        lyricsList.append(lineMap);
    }
    map["lyrics"] = lyricsList;

    return map;
}

QHash<int, QByteArray> SongModel::roleNames() const
{
    return
    {
        {TitleRole, "title"},
        {ArtistRole, "artist"},
        {AlbumRole, "album"},
        {DurationRole, "duration"},
        {FilePathRole, "filePath"},
        {UriRole, "uri"},
        {CoverRole, "songCover"},
        {SortRole, "sortKey"},
        {LyricsRole, "lyrics"}
    };
}

void SongModel::clear()
{
    beginResetModel();
    mSongList.clear();
    endResetModel();

    emit songCountChanged();
}

void SongModel::requestAndroidPermissions()
{
    int sdkVersion = QJniObject::getStaticField<jint>(
        "android/os/Build$VERSION", "SDK_INT"
        );

    qDebug() << "[Permissions] SDK_INT detected:" << sdkVersion;

    QStringList permissions;

    if (sdkVersion >= 33)
    {
        // Android 13+ (API 33+): READ_MEDIA_AUDIO
        permissions << "android.permission.READ_MEDIA_AUDIO";
        permissions << "android.permission.MANAGE_MEDIA";
    }
    else if (sdkVersion >= 30)
    {
        // Android 11-12 (API 30-32)
        permissions << "android.permission.READ_EXTERNAL_STORAGE";
        permissions << "android.permission.MANAGE_MEDIA";
    }
    else
    {
        // Android 10 and below (API <= 29)
        permissions << "android.permission.READ_EXTERNAL_STORAGE";
        permissions << "android.permission.WRITE_EXTERNAL_STORAGE";
    }

    bool allGranted = true;
    for (int i=0; i<permissions.size(); ++i)
    {
        const QString &permission = permissions[i];
        auto r = QtAndroidPrivate::checkPermission(permission).result();
        if (r != QtAndroidPrivate::Authorized)
        {
            r = QtAndroidPrivate::requestPermission(permission).result();
            if (r != QtAndroidPrivate::Authorized)
            {
                allGranted = false;
                emit permissionsDenied();
            }
        }
    }

    if (allGranted)
        emit permissionsGranted();
}

void SongModel::scanMusicFolder()
{
    clear();

    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject arrayList = QJniObject::callStaticObjectMethod(
        "com/amazzera/euterpe/MediaStoreReader",
        "scanMediaStoreSongs",
        "(Landroid/content/Context;)Ljava/util/ArrayList;",
        activity.object<jobject>()
        );

    if (!arrayList.isValid()) {
        qDebug() << "[MediaStore] arrayList non valido";
        return;
    }

    auto getStringSafe = [](QJniObject obj, const char* field) -> QString {
        QJniObject val = obj.getObjectField<jstring>(field);
        return val.isValid() ? val.toString() : "";
    };

    int size = arrayList.callMethod<jint>("size");
    for (int i = 0; i < size; ++i)
    {
        QJniObject jSong = arrayList.callObjectMethod("get","(I)Ljava/lang/Object;",i);

        if (!jSong.isValid())
            continue;

        QString filePath = getStringSafe(jSong, "filePath");
        QString content  = getStringSafe(jSong, "contentUri");
        QString title  = getStringSafe(jSong, "title");
        QString artist = getStringSafe(jSong, "artist");
        QString album  = getStringSafe(jSong, "album");
        QString cover  = getStringSafe(jSong, "coverPath");
        jlong duration = jSong.getField<jlong>("duration");

        Song currentSong(filePath.toStdString());

        currentSong.setContentUri(content.toStdString());
        currentSong.setTitle(title.toStdString());
        currentSong.setArtist(artist.toStdString());
        currentSong.setAlbum(album.toStdString());
        currentSong.setDuration(duration);
        currentSong.setSongCover(cover.toStdString());

        beginInsertRows(QModelIndex(), mSongList.size(), mSongList.size());
        mSongList.push_back(currentSong);
        endInsertRows();
    }

    if (!mJavaWatcher.isValid())
    {
        QJniObject activity = QNativeInterface::QAndroidApplication::context();

        mJavaWatcher = QJniObject("com/amazzera/euterpe/MediaStoreWatcher",
                                   "(Landroid/content/Context;)V",
                                   activity.object<jobject>());

        if (mJavaWatcher.isValid()) {
            mJavaWatcher.callMethod<void>("start");
        }
    }

    emit songCountChanged();
}

extern "C" JNIEXPORT void JNICALL
Java_com_amazzera_euterpe_MediaStoreWatcher_onMediaStoreChanged(JNIEnv *env, jclass clazz) {
    qDebug() << "[Euterpe] Cambio rilevato nel database Android!";

    if (s_instance) {
        QMetaObject::invokeMethod(s_instance, "scanMusicFolder", Qt::QueuedConnection);
    }
}

void SongModel::updateSongMetadataAt(int row)
{
    if (row < 0 || row >= static_cast<int>(mSongList.size()))
        return;

    Song &song = mSongList[row];
    QString filePath = QString::fromStdString(song.getFilePath());

    TagLib::FileRef f(filePath.toUtf8().constData());
    if (!f.isNull() && f.tag())
    {
        auto tag = f.tag();
        song.setTitle(tag->title().to8Bit(true));
        song.setArtist(tag->artist().to8Bit(true));
        song.setAlbum(tag->album().to8Bit(true));

        TagLib::MPEG::File mpegFile(filePath.toUtf8().constData());
        if (mpegFile.isValid() && mpegFile.ID3v2Tag()) {
            TagLib::ID3v2::Tag *id3vTag = mpegFile.ID3v2Tag();
            TagLib::ID3v2::FrameList lyricsFrames = id3vTag->frameList("USLT");

            if (!lyricsFrames.isEmpty()) {
                auto *lyricFrame = static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(lyricsFrames.front());
                if (lyricFrame) {
                    std::string raw = lyricFrame->text().to8Bit(true);
                    song.setLyrics(parseRawLyrics(raw));
                }
            } else {
                song.setLyrics({});
            }
        }

        QModelIndex idx = index(row, 0);
        emit dataChanged(idx, idx, {TitleRole, ArtistRole, AlbumRole, LyricsRole, SortRole});
    }
}

std::vector<Song::LyricLine> SongModel::parseRawLyrics(const std::string& raw)
{
    std::vector<Song::LyricLine> lines;
    QString qRaw = QString::fromStdString(raw);
    QStringList rawLines = qRaw.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);

    QRegularExpression timeRe("\\[(\\d+):(\\d+)(?:[:.](\\d+))?\\]");

    for (const QString& line : rawLines)
    {
        QString trimmed = line.trimmed();

        if (trimmed.startsWith("[") && trimmed.length() > 1 && trimmed.at(1).isLetter()) {
            qDebug() << "[Parser] Salto Tag ID3:" << trimmed;
            continue;
        }

        auto it = timeRe.globalMatch(trimmed);

        QString textOnly = trimmed;
        textOnly.remove(timeRe).trimmed();

        bool foundAtLeastOne = false;
        while (it.hasNext()) {
            auto match = it.next();
            foundAtLeastOne = true;

            QString originalTag = match.captured(0);

            long long mins = match.captured(1).toLongLong();
            long long secs = match.captured(2).toLongLong();
            QString msecsStr = match.captured(3);
            long long msecs = msecsStr.toLongLong();

            if (msecsStr.length() == 2) msecs *= 10;
            else if (msecsStr.length() == 1) msecs *= 100;

            long long totalMs = (mins * 60000LL) + (secs * 1000LL) + msecs;

            if (!textOnly.isEmpty()) {
                qDebug() << "[Parser] Match:" << originalTag
                         << "-> Convertito:" << totalMs << "ms"
                         << "| Testo:" << textOnly;
                lines.push_back({totalMs, textOnly.toStdString()});
            }
        }

        if (!foundAtLeastOne && !trimmed.isEmpty() && !trimmed.startsWith("[")) {
            qDebug() << "[Parser] Riga senza tempo:" << trimmed;
            lines.push_back({-1, trimmed.toStdString()});
        }
    }

    std::sort(lines.begin(), lines.end(), [](const Song::LyricLine& a, const Song::LyricLine& b) {
        return a.timeMs < b.timeMs;
    });

    qDebug() << "--- FINE PARSING (Totale righe:" << lines.size() << ") ---";

    return lines;
}

void SongModel::updateFullMetadata(const QString &path, const QString &t, const QString &ar,
                                   const QString &al, const QString &ly, const QString &cover)
{
    std::string targetPath = path.toStdString();

    for (int i = 0; i < static_cast<int>(mSongList.size()); ++i)
    {
        if (mSongList[i].getFilePath() == targetPath)
        {
            if (!t.isEmpty()) mSongList[i].setTitle(t.toStdString());
            if (!ar.isEmpty()) mSongList[i].setArtist(ar.toStdString());
            if (!al.isEmpty()) mSongList[i].setAlbum(al.toStdString());
            if (!cover.isEmpty()) mSongList[i].setSongCover(cover.toStdString());
            if (!ly.isEmpty()) mSongList[i].setLyrics(parseRawLyrics(ly.toStdString()));

            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {TitleRole, ArtistRole, AlbumRole, LyricsRole, CoverRole});

            return;
        }
    }
}

void SongModel::deleteSong(const QString &filePath)
{
    std::string target = filePath.toStdString();

    for (int i = 0; i < mSongList.size(); ++i)
    {
        if (mSongList[i].getFilePath() == target)
        {
            beginRemoveRows(QModelIndex(), i, i);
            mSongList.erase(mSongList.begin() + i);
            endRemoveRows();

            QFile file(filePath);
            if (file.remove()) {
                qDebug() << "File rimosso dal filesystem:" << filePath;

                QString coverPath = QFileInfo(filePath).path() + "/" +
                                    QFileInfo(filePath).completeBaseName() + ".jpg";
                QFile::remove(coverPath);
            }

            return;
        }
    }
}
