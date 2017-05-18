package com.example.chris.imagebender;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Point;
import android.view.Display;

import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Created by chris on 12/13/16.
 */

public class CSApp {
    public int screenWidth;
    public int screenHeight;
    //dangerous to keep around pointers to activities, but ill deal with that later
    public static MainActivity mainActivity;

    public CSApp(MainActivity a) {
        mainActivity = a;
        getScreenDimensions();
        generateInitialSeed();
    }

    public CSPrefs getPrefs() {
        return mainActivity.csPrefs;
    }

    private static native long generateInitialSeed();

    public void threadedToast(final String message) {
        mainActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mainActivity.showToast(message);
            }
        });
    }

    public void getScreenDimensions() {
        Display display = mainActivity.getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        screenWidth = size.x;
        screenHeight = size.y;
    }

    public static native void notifyContextChange(Context c);

    //called from jni
    public static void kill() {
        //System.exit(0);
        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                android.os.Process.killProcess(android.os.Process.myPid());
            }

        };
        Timer opening = new Timer();
        opening.schedule(task, 1000);

    }
}
