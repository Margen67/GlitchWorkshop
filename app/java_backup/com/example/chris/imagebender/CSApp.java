package com.example.chris.imagebender;

import android.app.Activity;
import android.graphics.Point;
import android.view.Display;

import java.util.Random;

/**
 * Created by chris on 12/13/16.
 */

public class CSApp {
    public int screenWidth;
    public int screenHeight;
    public static String cacheDir;
    private Random rng;
    //dangerous to keep around pointers to activities, but ill deal with that later
    public static MainActivity mainActivity;

    public int getScreenWidth() {
        return screenWidth;
    }

    public int getScreenHeight() {
        return screenHeight;
    }

    public CSApp(MainActivity a) {
        mainActivity = a;
        getScreenDimensions();
        rng = new Random();
        rng.setSeed(generateInitialSeed());
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

    public Random getRng() {
        return rng;
    }
}
