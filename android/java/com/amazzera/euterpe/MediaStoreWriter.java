package com.amazzera.euterpe;

import android.app.Activity;
import android.content.Intent;
import android.app.PendingIntent;
import android.content.ContentValues;
import android.content.ContentResolver;
import android.content.Context;
import android.content.IntentSender;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.util.Log;
import android.media.MediaScannerConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.database.Cursor;
import android.content.ContentUris;
import java.util.ArrayList;
import java.util.List;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;

public class MediaStoreWriter
{
    private static final String TAG = "MediaStoreWriter";
    public static final int PICK_IMAGE_REQUEST = 1002;

    public static void deleteFile(Context context, String filePath)
    {
        ContentResolver resolver = context.getContentResolver();
        Uri uri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
        String selection = MediaStore.Audio.Media.DATA + "=?";
        String[] selectionArgs = new String[]{ filePath };

        int deletedRows = resolver.delete(uri, selection, selectionArgs);
        Log.d(TAG, "MediaStore: removed " + deletedRows + " rows for " + filePath);

        cleanupOrphanedRecords(context);
    }

    public static void cleanupOrphanedRecords(Context context)
    {
        ContentResolver resolver = context.getContentResolver();
        Uri uri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
        String[] projection = { MediaStore.Audio.Media._ID, MediaStore.Audio.Media.DATA };

        try (Cursor cursor = resolver.query(uri, projection, null, null, null))
        {
            if (cursor != null)
            {
                while (cursor.moveToNext())
                {
                    String path = cursor.getString(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.DATA));
                    java.io.File file = new java.io.File(path);

                    if (!file.exists())
                    {
                        long id = cursor.getLong(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media._ID));
                        Uri deleteUri = ContentUris.withAppendedId(uri, id);
                        resolver.delete(deleteUri, null, null);
                        Log.d(TAG, "Removed ghost record: " + path);
                    }
                }
            }
        }
    }

    public static boolean updateSongMetadata(Context context, String uriStr, String title, String artist, String album)
    {
        Log.d(TAG, "Updating MediaStore database for: " + title);
        try {
            Uri uri = Uri.parse(uriStr);
            ContentValues values = new ContentValues();
            values.put(MediaStore.Audio.Media.TITLE, title);
            values.put(MediaStore.Audio.Media.ARTIST, artist);
            values.put(MediaStore.Audio.Media.ALBUM, album);

            int rows = context.getContentResolver().update(uri, values, null, null);
            Log.d(TAG, "Rows updated in DB: " + rows);

            refreshMetadata(context, uriStr);
            return rows > 0;
        } catch (SecurityException se) {
            Log.e(TAG, "Database Security Error: triggering permission request");
            requestWritePermission(context, uriStr);
            return false;
        } catch (Exception e) {
            Log.e(TAG, "Database Error: " + e.getMessage());
            return false;
        }
    }

    public static int getWriterFd(Context context, String uriStr)
    {
        Log.d(TAG, "Attempting to open FD for URI: " + uriStr);
        try {
            Uri uri = Uri.parse(uriStr);
            ParcelFileDescriptor pfd = context.getContentResolver().openFileDescriptor(uri, "rw");
            if (pfd != null) {
                int fd = pfd.detachFd();
                Log.d(TAG, "FD obtained: " + fd);
                return fd;
            }
        } catch (SecurityException se) {
            Log.e(TAG, "FD Security Error: triggering permission request");
            requestWritePermission(context, uriStr);
        } catch (Exception e) {
            Log.e(TAG, "FD Error: " + e.getMessage());
        }
        return -1;
    }

    public static void requestWritePermission(Context context, String uriStr)
    {
        if (!(context instanceof Activity))
            return;
        final Activity activity = (Activity) context;
        final Uri uri = Uri.parse(uriStr);

        activity.runOnUiThread(() -> {
            try {
                if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
                    List<Uri> uris = new ArrayList<>();
                    uris.add(uri);
                    PendingIntent pi = MediaStore.createWriteRequest(activity.getContentResolver(), uris);
                    activity.startIntentSenderForResult(pi.getIntentSender(), 1001, null, 0, 0, 0);
                    Log.d(TAG, "Permission popup launched (Android 11+)");
                } else if (android.os.Build.VERSION.SDK_INT == android.os.Build.VERSION_CODES.Q) {
                    Log.d(TAG, "Android 10 detected. Check requestLegacyExternalStorage in Manifest.");
                }
            } catch (Exception e) {
                Log.e(TAG, "Error launching permission popup: " + e.getMessage());
            }
        });
    }

    public static byte[] processSelectedImage(Context context, Uri imageUri)
    {
        try (InputStream is = context.getContentResolver().openInputStream(imageUri))
        {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeStream(is, null, options);

            options.inSampleSize = calculateInSampleSize(options, 800, 800);
            options.inJustDecodeBounds = false;

            try (InputStream isFinal = context.getContentResolver().openInputStream(imageUri)) {
                Bitmap bitmap = BitmapFactory.decodeStream(isFinal, null, options);

                int width = bitmap.getWidth();
                int height = bitmap.getHeight();
                float ratio = (float) width / height;

                int targetWidth = 800;
                int targetHeight = 800;

                if (width > height) targetHeight = Math.round(targetWidth / ratio);
                else targetWidth = Math.round(targetHeight * ratio);

                Bitmap scaled = Bitmap.createScaledBitmap(bitmap, targetWidth, targetHeight, true);
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                scaled.compress(Bitmap.CompressFormat.JPEG, 80, baos);

                byte[] bytes = baos.toByteArray();

                bitmap.recycle();
                scaled.recycle();
                return bytes;
            }
        }
        catch (Exception e)
        {
            Log.e(TAG, "Error processing image: " + e.getMessage());
            return null;
        }
    }

    private static int calculateInSampleSize(BitmapFactory.Options options, int reqWidth, int reqHeight)
    {
        final int height = options.outHeight;
        final int width = options.outWidth;
        int inSampleSize = 1;
        if (height > reqHeight || width > reqWidth)
        {
            final int halfHeight = height / 2;
            final int halfWidth = width / 2;
            while ((halfHeight / inSampleSize) >= reqHeight && (halfWidth / inSampleSize) >= reqWidth)
            {
                inSampleSize *= 2;
            }
        }
        return inSampleSize;
    }

    public static String readTextFromUri(Context context, Uri uri)
    {
        Log.d(TAG, "Attempting to read text from URI: " + uri.toString());
        StringBuilder stringBuilder = new StringBuilder();

        try (InputStream inputStream = context.getContentResolver().openInputStream(uri);
             java.io.BufferedReader reader = new java.io.BufferedReader(
                     new java.io.InputStreamReader(inputStream, java.nio.charset.StandardCharsets.UTF_8))) {
            String line;
            while ((line = reader.readLine()) != null) {
                stringBuilder.append(line).append("\n");
            }
            Log.d(TAG, "Read completed. Characters: " + stringBuilder.length());
            return stringBuilder.toString();
        }
        catch (Exception e)
        {
            Log.e(TAG, "Error reading text file: " + e.getMessage());
            return null;
        }
    }

    private static void refreshMetadata(Context context, String uriStr)
    {
        try
        {
            Uri uri = Uri.parse(uriStr);
            context.getContentResolver().notifyChange(uri, null);
            String[] paths = { uri.getPath() };
            MediaScannerConnection.scanFile(context, paths, null, (path, scanUri) -> {
                Log.d(TAG, "Scan completed for: " + path);
            });
        }
        catch (Exception e)
        {
            Log.e(TAG, "Refresh error: " + e.getMessage());
        }
    }
}
