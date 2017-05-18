package com.example.chris.imagebender;

import android.app.Activity;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

/**
 * Created by chris on 12/18/16.
 */

public class CSPrefs {
    private SharedPreferences prefs;
    CSPrefs(Activity mainActivity) {
        prefs = PreferenceManager.getDefaultSharedPreferences(mainActivity);
    }

    void putInt(String key, int value) {
        SharedPreferences.Editor editor = prefs.edit();
        editor.putInt(key, value);
        editor.apply();
    }
    //our default value is always -1
    int getInt(String key) {
        return prefs.getInt(key, -1);
    }
}
