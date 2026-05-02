package com.amazzera.euterpe;

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.provider.MediaStore;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.util.ArrayList;

public class MediaStoreReader {
    private static final String TAG = "MediaStoreReader";

    public static ArrayList<SongInfo> scanMediaStoreSongs(Context context) {
        ArrayList<SongInfo> list = new ArrayList<>();

        try {
            Uri collection = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;

            String[] projection = {
                    MediaStore.Audio.Media._ID,
                    MediaStore.Audio.Media.DATA,
                    MediaStore.Audio.Media.TITLE,
                    MediaStore.Audio.Media.ARTIST,
                    MediaStore.Audio.Media.ALBUM,
                    MediaStore.Audio.Media.DURATION
            };

            try (Cursor cursor = context.getContentResolver().query(
                    collection,
                    projection,
                    MediaStore.Audio.Media.IS_MUSIC + " != 0",
                    null,
                    MediaStore.Audio.Media.TITLE + " ASC")) {

                if (cursor != null) {
                    int idCol = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media._ID);
                    int pathCol = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.DATA);
                    int msTitleCol = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.TITLE);
                    int msArtistCol = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ARTIST);
                    int msAlbumCol = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ALBUM);

                    while (cursor.moveToNext()) {
                        String filePath = cursor.getString(pathCol);

                        if (filePath == null || !filePath.toLowerCase().endsWith(".mp3")) continue;
                        if (!filePath.contains("/Music/")) continue;

                        long id = cursor.getLong(idCol);
                        SongInfo song = new SongInfo();
                        song.contentUri = ContentUris.withAppendedId(collection, id).toString();
                        song.filePath = filePath;

                        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
                        try {
                            retriever.setDataSource(context, Uri.parse(song.contentUri));

                            String title = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
                            String artist = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);
                            String album = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ALBUM);
                            String durationStr = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
                            byte[] art = retriever.getEmbeddedPicture();

                            if (title == null || title.trim().isEmpty()) {
                                title = cursor.getString(msTitleCol);
                                if (title == null || title.trim().isEmpty()) {
                                    title = getCleanFileName(filePath);
                                }
                            }
                            song.title = title;

                            if (artist == null || artist.trim().isEmpty() || artist.equalsIgnoreCase("<unknown>")) {
                                artist = cursor.getString(msArtistCol);
                                if (artist == null || artist.trim().isEmpty() || artist.equalsIgnoreCase("<unknown>")) {
                                    artist = "Unknown Artist";
                                }
                            }
                            song.artist = artist;

                            if (album == null || album.trim().isEmpty()) {
                                album = cursor.getString(msAlbumCol);
                                if (album == null || album.trim().isEmpty()) {
                                    album = "Unknown Album";
                                }
                            }
                            song.album = album;

                            song.duration = (durationStr != null) ? Long.parseLong(durationStr) : 0;

                            if (art != null) {
                                File cacheDir = context.getCacheDir();
                                String fileName = "cover_" + String.valueOf(song.filePath.hashCode()) + ".jpg";
                                File file = new File(cacheDir, fileName);

                                if (!file.exists()) {
                                    try (FileOutputStream fos = new FileOutputStream(file)) {
                                        fos.write(art);
                                    }
                                }
                                song.coverPath = file.getAbsolutePath();
                            } else {
                                song.coverPath = "";
                            }

                        } catch (Exception e) {
                            Log.e(TAG, "Metadata error for: " + filePath);
                        } finally {
                            retriever.release();
                        }

                        list.add(song);
                    }
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Error reading MediaStore: " + e.getMessage());
        }

        return list;
    }

    private static String getCleanFileName(String filePath) {
        try {
            File file = new File(filePath);
            String name = file.getName();
            int lastDot = name.lastIndexOf('.');
            if (lastDot > 0) name = name.substring(0, lastDot);
            return name.replace("_", " ").trim();
        } catch (Exception e) {
            return "Unknown Track";
        }
    }
}
