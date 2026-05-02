package com.amazzera.euterpe;

import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.provider.MediaStore;
import android.util.Log;

public class MediaStoreWatcher extends ContentObserver
{
    private static final String TAG = "MediaStoreWatcher";
    private final Context mContext;
    private final Handler mHandler;
    private Runnable mPendingRefresh = null;
    private static final int DEBOUNCE_DELAY_MS = 1000;

    public static native void onMediaStoreChanged();

    public MediaStoreWatcher(Context context)
    {
        super(new Handler(Looper.getMainLooper()));
        this.mContext = context;
        this.mHandler = new Handler(Looper.getMainLooper());
    }

    public void start()
    {
        try
        {
            Uri audioUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
            mContext.getContentResolver().registerContentObserver(audioUri, true, this);
            Log.d(TAG, "MediaStore monitoring started successfully.");
        }
        catch (Exception e)
        {
            Log.e(TAG, "Error registering Watcher: " + e.getMessage());
        }
    }

    public void stop()
    {
        try
        {
            mContext.getContentResolver().unregisterContentObserver(this);
            mHandler.removeCallbacksAndMessages(null);
            mPendingRefresh = null;
            Log.d(TAG, "MediaStore monitoring stopped.");
        }
        catch (Exception e)
        {
            Log.e(TAG, "Error removing Watcher: " + e.getMessage());
        }
    }

    @Override
    public void onChange(boolean selfChange, Uri uri)
    {
        super.onChange(selfChange, uri);

        if (selfChange) return;

        Log.d(TAG, "MediaStore change detected: " + (uri != null ? uri.toString() : "all"));

        if (mPendingRefresh != null) {
            mHandler.removeCallbacks(mPendingRefresh);
        }

        mPendingRefresh = () -> {
            Log.d(TAG, "Debounce finished. Notifying C++ bridge...");
            onMediaStoreChanged();
        };

        mHandler.postDelayed(mPendingRefresh, DEBOUNCE_DELAY_MS);
    }
}
