package com.example.chris.imagebender;


import java.util.Random;

/**
 * Created by chris on 12/12/16.
 */
class VideoProcessingThread extends Thread {
    private final String videopath;

    public VideoProcessingThread(String videopath, Random rng) {
        this.videopath = videopath;

    }

    private native void runNative(String videopath);

    public void run() {

        runNative(videopath);
        MainActivity.videoBendingThread = null;

    }

    public static native void notifyHaveSavename(String savename);

}
