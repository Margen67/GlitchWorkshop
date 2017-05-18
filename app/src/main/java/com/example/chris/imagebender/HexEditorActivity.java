package com.example.chris.imagebender;

import android.content.Context;
import android.support.v7.app.AppCompatActivity;

/**
 * Created by chris on 2/15/17.
 */

public class HexEditorActivity extends AppCompatActivity {
    @Override
    public void onResume() {
        super.onResume();
        CSApp.notifyContextChange(this);
    }
}
