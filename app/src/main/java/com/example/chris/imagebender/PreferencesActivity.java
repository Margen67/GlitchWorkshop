package com.example.chris.imagebender;

import android.content.Context;
import android.graphics.Bitmap;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.Spinner;

public class PreferencesActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_preferences);
        Spinner spinOutputFileFormat = (Spinner) findViewById(R.id.spinOutputFormat);
        spinOutputFileFormat.setSelection(MainActivity.outputFormat.ordinal());
        spinOutputFileFormat.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                Bitmap.CompressFormat newformat = null;
                for(Bitmap.CompressFormat f : Bitmap.CompressFormat.values()) {
                    if (f.ordinal() == position) {
                        newformat = f;
                        break;
                    }
                }

                MainActivity.outputFormat = newformat;
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        CheckBox chkPreserveTrans = (CheckBox)findViewById(R.id.chkPreserveTrans);
        chkPreserveTrans.setChecked(GlitchAlgo.getPreserveTransparency());
        CheckBox chkRememberLast = (CheckBox)findViewById(R.id.chkRememberLast);
    }
    @Override
    public void onResume() {
        super.onResume();
        CSApp.notifyContextChange((Context) this);
    }
}
