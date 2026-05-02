package com.amazzera.euterpe;

public class SongInfo {

    public String contentUri;
    public String filePath;

    public String title;
    public String artist;
    public String album;
    public long duration;

    public String coverPath;

    public SongInfo() {}

    public SongInfo(String contentUri,
                    String filePath,
                    String title,
                    String artist,
                    String album,
                    long duration,
                    String coverPath) {

        this.contentUri = contentUri;
        this.filePath = filePath;
        this.title = title;
        this.artist = artist;
        this.album = album;
        this.duration = duration;
        this.coverPath = coverPath;
    }
}
