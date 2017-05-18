package com.example.chris.imagebender;


import android.app.ProgressDialog;

import java.util.Random;

/**
 * Created by chris on 12/12/16.
 */
class VideoProcessingThread extends Thread {
    private final String videopath;
    ProgressDialog dlg;
    public VideoProcessingThread(String videopath, MainActivity activity) {
        this.videopath = videopath;
        dlg = ProgressDialog.show(activity, "Generating glitched video", "This might take a while...", true);
    }

    private native void runNative(String videopath);

    public void run() {

        runNative(videopath);
        MainActivity.videoBendingThread = null;
        if(dlg != null)
            dlg.dismiss();
    }

    public static native void notifyHaveSavename(String savename);

}
