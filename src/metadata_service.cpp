#include <unistd.h>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QSysInfo>
#include <QFileInfo>
#include <QJniObject>
#include <QtCore/qcoreapplication.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tag.h>
#include <fileref.h>
#include <toolkit/tfilestream.h>
#include <toolkit/tbytevector.h>
#include <toolkit/tiostream.h>
#include <toolkit/tstring.h>
#include <mpeg/mpegfile.h>
#include <mpeg/id3v2/frames/attachedpictureframe.h>
#include <mpeg/id3v2/frames/unsynchronizedlyricsframe.h>
#include <mpeg/id3v2/id3v2tag.h>
#include <mpeg/id3v2/id3v2frame.h>

#include "include/metadata_service.h"

void MetadataService::setInitialState(const QString& title, const QString& artist, const QString& album)
{
    mPendingTitleData=title;
    mPendingArtistData=artist;
    mPendingAlbumData=album;
}

void MetadataService::setMetadataTitle(const QString& title)
{
    if(mPendingTitleData!=title)
        mPendingTitleData=title;
    else
        mPendingTitleData=std::nullopt;
}

void MetadataService::setMetadataArtist(const QString& artist)
{
    if(mPendingArtistData!=artist)
        mPendingArtistData=artist;
    else
        mPendingArtistData=std::nullopt;
}

void MetadataService::setMetadataAlbum(const QString& album)
{
    if(mPendingAlbumData!=album)
        mPendingAlbumData=album;
    else
        mPendingAlbumData=std::nullopt;
}

void MetadataService::pickCoverImage()
{
    QJniObject action = QJniObject::fromString("android.intent.action.GET_CONTENT");
    QJniObject intent("android/content/Intent", "(Ljava/lang/String;)V", action.object());
    intent.callObjectMethod("setType",
                            "(Ljava/lang/String;)Landroid/content/Intent;",
                            QJniObject::fromString("image/*").object()
                            );
    QtAndroidPrivate::startActivity(intent, 1002, this);
}

void MetadataService::pickLyricsFile()
{
    QJniObject action = QJniObject::fromString("android.intent.action.GET_CONTENT");
    QJniObject intent("android/content/Intent", "(Ljava/lang/String;)V", action.object());

    // Filter for text and generic file (such as .lrc)
    intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;",
                            QJniObject::fromString("text/*").object());

    QtAndroidPrivate::startActivity(intent, 1003, this);
}

void MetadataService::updateMetadata(const QString& filePath, const QString& uri)
{
    qDebug() << "[MetadataService] Starting write procedure";

    bool hasChanges = (mPendingTitleData || mPendingArtistData || mPendingAlbumData ||
                       !mPendingCoverData.isEmpty() || !mPendingLyricsText.isEmpty());

    if (!hasChanges)
    {
        qDebug() << "[MetadataService] ⚠️ No actual changes detected.";

        // Force a safety reset to prevent any empty strings ‘’ from being interpreted as changes on the second click.
        mPendingTitleData.reset();
        mPendingArtistData.reset();
        mPendingAlbumData.reset();
        mPendingCoverData.clear();
        mPendingLyricsText.clear();

        emit metadataSaved(false, "", "", "", "", "", "");
        return;
    }

    // 1. Declare date of new metadata
    QString finalTitle  = mPendingTitleData ? *mPendingTitleData : "";
    QString finalArtist = mPendingArtistData ? *mPendingArtistData : "";
    QString finalAlbum  = mPendingAlbumData ? *mPendingAlbumData : "";
    QString generatedCoverPath = "";

    // 2. Clear URI
    QString cleanUri = uri.trimmed();
    if (cleanUri.startsWith("\"") && cleanUri.endsWith("\""))
    {
        cleanUri = cleanUri.mid(1, cleanUri.length() - 2);
    }

    // 3. Check SDK Version
    int sdkVersion = QJniObject::getStaticField<jint>("android/os/Build$VERSION", "SDK_INT");
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    // Update Metadata for ANDROID 10 (API 29) or less ---
    if (sdkVersion <= 29)
    {
        qDebug() << "[MetadataService] Detect SDK <= 29";

        TagLib::MPEG::File f(filePath.toLocal8Bit().constData());

        if (f.isValid() && f.ID3v2Tag())
        {
            TagLib::ID3v2::Tag *tag = f.ID3v2Tag();
            if (mPendingTitleData)  tag->setTitle(mPendingTitleData->toStdString());
            if (mPendingArtistData) tag->setArtist(mPendingArtistData->toStdString());
            if (mPendingAlbumData)  tag->setAlbum(mPendingAlbumData->toStdString());

            if (!mPendingCoverData.isEmpty())
            {
                tag->removeFrames("APIC");
                TagLib::ID3v2::AttachedPictureFrame *frame = new TagLib::ID3v2::AttachedPictureFrame;
                frame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
                frame->setMimeType("image/jpeg");
                frame->setPicture(TagLib::ByteVector(mPendingCoverData.constData(), mPendingCoverData.size()));
                tag->addFrame(frame);

                generatedCoverPath = filePath;
                generatedCoverPath = QFileInfo(filePath).path() + "/" +
                                     QFileInfo(filePath).completeBaseName() + ".jpg";
                QFile imageFile(generatedCoverPath);
                if (imageFile.open(QIODevice::WriteOnly)) {
                    imageFile.write(mPendingCoverData);
                    imageFile.close();
                    qDebug() << "[MetadataService] Cover exported to:" << generatedCoverPath;
                }
            }

            if (!mPendingLyricsText.isEmpty())
            {
                qDebug() << "[MetadataService] Inserting USLT Frame (Lyrics)...";

                tag->removeFrames("USLT");

                TagLib::ID3v2::UnsynchronizedLyricsFrame *lyricsFrame = new TagLib::ID3v2::UnsynchronizedLyricsFrame;

                lyricsFrame->setLanguage("XXX");
                lyricsFrame->setText(mPendingLyricsText.toStdString());
                lyricsFrame->setTextEncoding(TagLib::String::UTF8);

                tag->addFrame(lyricsFrame);

                qDebug() << "[MetadataService] Lyrics text injected successfully.";
            }

            if (f.save())
            {
                qDebug() << "[MetadataService] ✅ Direct physical write successful!";

                QJniObject::callStaticMethod<jboolean>(
                    "com/amazzera/euterpe/MediaStoreWriter",
                    "updateSongMetadata",
                    "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
                    activity.object(),
                    QJniObject::fromString(cleanUri).object<jstring>(),
                    QJniObject::fromString(finalTitle).object<jstring>(),
                    QJniObject::fromString(finalArtist).object<jstring>(),
                    QJniObject::fromString(finalAlbum).object<jstring>()
                    );

                emit metadataSaved(true, filePath, finalTitle, finalArtist, finalAlbum, mPendingLyricsText, generatedCoverPath);

                mPendingTitleData.reset();
                mPendingArtistData.reset();
                mPendingAlbumData.reset();
                mPendingCoverData.clear();
                mPendingLyricsText.clear();

                return;
            }
            else
            {
                qWarning() << "[MetadataService] ❌ TagLib f.save() failed on direct path.";
            }
        }
        qWarning() << "[MetadataService] Direct write failed, trying FD method...";
    }
    else // Update Metadata for ANDROID 11 (API 30+)
    {
        qDebug() << "[MetadataService] Detect SDK 30+";

        QJniObject jUri = QJniObject::fromString(cleanUri);

        jint fd = QJniObject::callStaticMethod<jint>(
            "com/amazzera/euterpe/MediaStoreWriter",
            "getWriterFd",
            "(Landroid/content/Context;Ljava/lang/String;)I",
            activity.object(),
            jUri.object<jstring>()
            );

        if (fd <= 0) {
            qWarning() << "[MetadataService] ❌ FD not obtained. Requesting write permission...";
            QJniObject::callStaticMethod<void>(
                "com/amazzera/euterpe/MediaStoreWriter",
                "requestWritePermission",
                "(Landroid/content/Context;Ljava/lang/String;)V",
                activity.object(),
                jUri.object<jstring>()
                );
            emit metadataSaved(false, "", "", "", "", "", "");
            return;
        }

        qDebug() << "[MetadataService] 🛠 FD received:" << fd << ". Starting TagLib stream operation.";

        bool tagLibSuccess = false;
        {
            TagLib::FileStream stream(fd);
            TagLib::MPEG::File f(&stream);

            if (f.isValid() && f.ID3v2Tag()) {
                TagLib::ID3v2::Tag *tag = f.ID3v2Tag();

                if (mPendingTitleData)  tag->setTitle(mPendingTitleData->toStdString());
                if (mPendingArtistData) tag->setArtist(mPendingArtistData->toStdString());
                if (mPendingAlbumData)  tag->setAlbum(mPendingAlbumData->toStdString());

                // Aggiornamento Cover (APIC)
                if (!mPendingCoverData.isEmpty()) {
                    qDebug() << "[MetadataService] Inserting APIC Frame via FD...";
                    tag->removeFrames("APIC");
                    TagLib::ID3v2::AttachedPictureFrame *frame = new TagLib::ID3v2::AttachedPictureFrame;
                    frame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
                    frame->setMimeType("image/jpeg");
                    frame->setPicture(TagLib::ByteVector(mPendingCoverData.constData(), mPendingCoverData.size()));
                    tag->addFrame(frame);
                }

                if (!mPendingLyricsText.isEmpty()) {
                    qDebug() << "[MetadataService] Inserting USLT Frame via FD...";
                    tag->removeFrames("USLT");
                    TagLib::ID3v2::UnsynchronizedLyricsFrame *lyricsFrame = new TagLib::ID3v2::UnsynchronizedLyricsFrame;
                    lyricsFrame->setLanguage("XXX");
                    lyricsFrame->setText(mPendingLyricsText.toStdString());
                    lyricsFrame->setTextEncoding(TagLib::String::UTF8);
                    tag->addFrame(lyricsFrame);
                }

                tagLibSuccess = f.save();
            }
            else
            {
                qCritical() << "[MetadataService] ❌ TagLib: Unable to read tags from FD stream.";
            }
        }

        ::close(fd);
        qDebug() << "[MetadataService] FD closed. TagLib success:" << tagLibSuccess;

        if (tagLibSuccess) {
            jboolean dbSuccess = QJniObject::callStaticMethod<jboolean>(
                "com/amazzera/euterpe/MediaStoreWriter",
                "updateSongMetadata",
                "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
                activity.object(),
                jUri.object<jstring>(),
                QJniObject::fromString(finalTitle).object<jstring>(),
                QJniObject::fromString(finalArtist).object<jstring>(),
                QJniObject::fromString(finalAlbum).object<jstring>()
                );

            qDebug() << "[MetadataService] MediaStore DB Update:" << (dbSuccess ? "SUCCESSFUL" : "FAILED");

            if (!mPendingCoverData.isEmpty()) {
                generatedCoverPath = filePath;
                generatedCoverPath = QFileInfo(filePath).path() + "/" +
                                     QFileInfo(filePath).completeBaseName() + ".jpg";
                QFile imageFile(generatedCoverPath);
                if (imageFile.open(QIODevice::WriteOnly)) {
                    imageFile.write(mPendingCoverData);
                    imageFile.close();
                }
            }

            emit metadataSaved(true, filePath, finalTitle, finalArtist, finalAlbum, mPendingLyricsText, generatedCoverPath);
            clearPendingData();
        }
        else
        {
            clearPendingData();
            emit metadataSaved(false, "", "", "", "", "", "");
        }
    }
}

void MetadataService::handleActivityResult(int requestCode, int resultCode, const QJniObject &data)
{
    qDebug() << ">>>> [C++] handleActivityResult TRIGGERED! Code:" << requestCode << "Result:" << resultCode;

    if (requestCode == 1002 && resultCode == -1)
    {
        qDebug() << "[MetadataService] Acquire song cover from gallery";

        QJniObject uri = data.callObjectMethod("getData", "()Landroid/net/Uri;");
        if (uri.isValid())
        {
            qDebug() << "[MetadataService] URI obtained:" << uri.toString();

            QJniObject activity = QNativeInterface::QAndroidApplication::context();

            QJniObject bytesArray = QJniObject::callStaticMethod<jbyteArray>(
                "com/amazzera/euterpe/MediaStoreWriter",
                "processSelectedImage",
                "(Landroid/content/Context;Landroid/net/Uri;)[B",
                activity.object(),
                uri.object()
                );

            if (bytesArray.isValid())
            {
                jbyteArray jBytes = bytesArray.object<jbyteArray>();
                QJniEnvironment env;
                jsize len = env->GetArrayLength(jBytes);
                jbyte* body = env->GetByteArrayElements(jBytes, 0);

                mPendingCoverData = QByteArray((char*)body, len);

                env->ReleaseByteArrayElements(jBytes, body, JNI_ABORT);
                qDebug() << "[MetadataService] Cover Loaded. Bytes: " << mPendingCoverData.size();
            }
        }
    }
    else if (requestCode == 1003 && resultCode == -1)
    {
        qDebug() << "[MetadataService] Acquire song lyrics from file manager";

        QJniObject uri = data.callObjectMethod("getData", "()Landroid/net/Uri;");
        if (uri.isValid())
        {
            QJniObject activity = QNativeInterface::QAndroidApplication::context();
            QJniObject lrcText = QJniObject::callStaticMethod<jstring>(
                "com/amazzera/euterpe/MediaStoreWriter",
                "readTextFromUri",
                "(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;",
                activity.object(),
                uri.object()
                );

            if (lrcText.isValid())
            {
                mPendingLyricsText = lrcText.toString();
                qDebug() << "[MetadataService] ✅ Lyrics loaded. Characters:" << mPendingLyricsText.length();
            }
        }
    }
    else
    {
        qDebug() << "[MetadataService] Selection failed or cancelled. ResultCode:" << resultCode;
    }
}

void MetadataService::clearPendingData()
{
    mPendingTitleData.reset();
    mPendingArtistData.reset();
    mPendingAlbumData.reset();
    mPendingCoverData.clear();
    mPendingLyricsText.clear();
    qDebug() << "[MetadataService] Temporary buffers cleared.";
}

void MetadataService::removeSongFromAndroidDatabase(const QString &filePath)
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject jPath = QJniObject::fromString(filePath);

    QJniObject::callStaticMethod<void>(
        "com/amazzera/euterpe/MediaStoreWriter",
        "deleteFile",
        "(Landroid/content/Context;Ljava/lang/String;)V",
        activity.object(),
        jPath.object<jstring>()
        );
}
