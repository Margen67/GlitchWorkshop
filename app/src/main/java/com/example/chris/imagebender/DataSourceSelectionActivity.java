package com.example.chris.imagebender;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;

import android.provider.MediaStore;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import static com.example.chris.imagebender.DataSourceSelection.*;
import static com.example.chris.imagebender.MainActivity.IMAGE_OPENED_RESULT;

public class DataSourceSelectionActivity extends AppCompatActivity {
    private static final String keyDontShowHelp = "dont_show_data_source_help";

    private int oldWidgetId = -1;

    private void displayHelpText() {

        if(MainActivity.csApp.getPrefs().getInt(keyDontShowHelp) == 1)
            return;
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Information");
        final TextView input = new TextView(this);
        input.setText(getResources().getText(R.string.strDataChoiceExplanation));
        builder.setView(input);

        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        builder.setNegativeButton("Don't show again", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                MainActivity.csApp.getPrefs().putInt(keyDontShowHelp, 1);
            }
        });

        builder.show();
    }

    @Override
    public void onResume() {
        super.onResume();
        CSApp.notifyContextChange((Context) this);
    }



    private void removeOldWidget() {
        if(oldWidgetId != -1)
        {

            makeGone(oldWidgetId);
            oldWidgetId = -1;
        }
    }

    private void addNewWidget(int id) {
        removeOldWidget();
        oldWidgetId = id;
        findViewById(id).setVisibility(View.VISIBLE);
    }

    private void makeGone(int id) {
        View v = findViewById(id);
        v.setVisibility(View.GONE);
    }

    @Override
    protected void onStart() {
        super.onStart();
        makeGone(R.id.etConstantNumber);
        makeGone(R.id.btnLoadImageAsDataSource);
        makeGone(R.id.etPlaintext);

        switch(MainActivity.getAlgoObject().getSelectedDataSource()) {
            case SEL_RNG:
                break;
            case SEL_CONSTANT:
            {
                addNewWidget(R.id.etConstantNumber);
                EditText etConstant = (EditText) findViewById(R.id.etConstantNumber);
                etConstant.setText(Integer.toString(GlitchAlgo.getConstantValue()));
                break;
            }
            case SEL_TEXT:
            {
                addNewWidget(R.id.etPlaintext);
                EditText etPlaintext = (EditText) findViewById(R.id.etPlaintext);
                etPlaintext.setText(GlitchAlgo.getTextValue());
                break;
            }
            case SEL_IMAGE:
            {
                addNewWidget(R.id.btnLoadImageAsDataSource);
                break;
            }
        }
    }

    @Override
    public void onBackPressed() {
        int sourceChoice = MainActivity.getAlgoObject().getSelectedDataSource();

        //process whatever input we have
        switch(sourceChoice) {
            case SEL_RNG:
                break;
            case SEL_CONSTANT:
            {
                EditText etConstant = (EditText)findViewById(R.id.etConstantNumber);
                String enteredString = etConstant.getText().toString();
                if(enteredString == null || enteredString == "") {
                    Toast.makeText(this, "No number was entered. Defaulting to randomly generating numbers.", Toast.LENGTH_LONG).show();
                    MainActivity.getAlgoObject().setSelectedDataSource(SEL_RNG);
                    break;
                }
                int enteredVal = Integer.parseInt(enteredString);
                //wrap to byte max
                GlitchAlgo.setConstantValue(enteredVal % 255);
                break;
            }
            case SEL_IMAGE:
            {
                if(GlitchAlgo.getOriginalDataSrcBmp() == null) {
                    Toast.makeText(this, "No bitmap was selected as a data source. Defaulting to randomly generated numbers.", Toast.LENGTH_LONG).show();
                    MainActivity.getAlgoObject().setSelectedDataSource(SEL_RNG);
                }
                break;
            }
            case SEL_TEXT:
            {
                EditText etPlaintext = (EditText)findViewById(R.id.etPlaintext);
                String textVal = etPlaintext.getText().toString();
                if(textVal == null || textVal == "")
                {
                    Toast.makeText(this, "No text was entered. Defaulting to randomly generated numbers.", Toast.LENGTH_LONG).show();
                    MainActivity.getAlgoObject().setSelectedDataSource(SEL_RNG);
                    break;
                }
                GlitchAlgo.setTextValue(textVal);
                break;
            }
        }
        super.onBackPressed();
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent i) {
        if (resultCode == -1 && requestCode == IMAGE_OPENED_RESULT) {
            String path = FileUtils.readPathFromUri(this, i.getData());

            NativelyCachedBmp bmp = new NativelyCachedBmp(path);

            GlitchAlgo.setOriginalDataSrcBmp(bmp);

            if(MainActivity.originalBitmap != null) {
                NativelyCachedBmp scaled = new NativelyCachedBmp(bmp, MainActivity.originalBitmap.getW(), MainActivity.originalBitmap.getH());
                GlitchAlgo.setResizedDataSrcBmp(scaled);
            }

        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_data_source_selection);

        RadioGroup rgSourceSelection = (RadioGroup)findViewById(R.id.rgSourceSelection);

        rgSourceSelection.check(rgSourceSelection.getChildAt(GlitchAlgo.getSelectedDataSource()).getId());
        displayHelpText();

        rgSourceSelection.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup radioGroup, int i) {
                RadioButton selectedBtn = (RadioButton) findViewById(i);
                int selectedSource = radioGroup.indexOfChild(selectedBtn);
                int oldSource = MainActivity.getAlgoObject().getSelectedDataSource();
                switch(oldSource) {
                    case SEL_RNG:
                        break;
                    case SEL_CONSTANT:
                        GlitchAlgo.setConstantValue(0);
                        break;
                    case SEL_IMAGE:
                        GlitchAlgo.setResizedDataSrcBmp(null);
                        GlitchAlgo.setOriginalDataSrcBmp(null);
                        break;
                    case SEL_TEXT:
                        GlitchAlgo.setTextValue(null);
                        break;
                }

                switch(selectedSource) {
                    case SEL_RNG:
                        removeOldWidget();
                        break;
                    case SEL_CONSTANT:
                        addNewWidget(R.id.etConstantNumber);
                        break;
                    case SEL_TEXT:
                        addNewWidget(R.id.etPlaintext);
                        break;
                    case SEL_IMAGE:
                        addNewWidget(R.id.btnLoadImageAsDataSource);
                        break;

                }

                MainActivity.getAlgoObject().setSelectedDataSource(selectedSource);
            }
        });

        Button btnLoadImageAsDataSource = (Button)findViewById(R.id.btnLoadImageAsDataSource);
        btnLoadImageAsDataSource.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent fileSaveIntent = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                try {
                    startActivityForResult(Intent.createChooser(fileSaveIntent, "Select an image"), IMAGE_OPENED_RESULT);
                } catch(ActivityNotFoundException e) {
                    Toast.makeText(DataSourceSelectionActivity.this, "Please install a file manager.", Toast.LENGTH_LONG).show();
                }
            }
        });
    }
}
