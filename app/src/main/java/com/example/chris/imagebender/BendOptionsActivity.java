package com.example.chris.imagebender;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.ToggleButton;
import static com.example.chris.imagebender.GlitchAlgo.*;
public class BendOptionsActivity extends AppCompatActivity {
    @Override
    public void onResume() {
        super.onResume();
        CSApp.notifyContextChange((Context) this);
    }
    private void hideAlgo(int id) {
        View v = findViewById(id);
        v.setVisibility(View.GONE);
    }
    private void showAlgo(int id) {
        View v = findViewById(id);
        v.setVisibility(View.VISIBLE);
    }
    private static final int hiddens[] =
    {
            R.id.pixelSortChoice,
            R.id.revXorChoice,
            R.id.rbyteChoice
    };
    private static final int showns[] = {
            R.id.spinForceFormat,
            R.id.txtForcedImageFmt
    };
    private static final int unaryOps[] = {
        ALGO_PIXELSORT,
        ALGO_RBIT,
        ALGO_RBYTE
    };
    private static boolean isUnary(int i) {
        for(int j = 0; j < unaryOps.length; ++j)
            if(unaryOps[j] == i)
                return true;
        return false;
    }
    private void displayRawDataWarning(final ToggleButton tB) {

        if(MainActivity.csApp.getPrefs().getInt("nowarn_rawdata") == 1)
            return;
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Warning");
        final TextView input = new TextView(this);
        input.setText(getResources().getText(R.string.strRawDataWarning));
        builder.setView(input);

        builder.setItems(new CharSequence[] {
                "OK",
                "Cancel",
                "Don't show again"
        }, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case 0:
                        break;
                    case 1:
                        tB.setChecked(false);
                        GlitchAlgo.setDatamode(0);
                        for(int i = 0; i < hiddens.length; ++i)
                            showAlgo(hiddens[i]);
                        break;
                    case 2:
                        MainActivity.csApp.getPrefs().putInt("nowarn_rawdata", 1);
                        break;
                }
            }
        });

        builder.show();
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bend_options);

        RadioGroup rg = (RadioGroup) findViewById(R.id.groupAlgoChoices);
        if(getAlgoChoice() != ALGO_NONE)
            rg.check(rg.getChildAt(getAlgoChoice() - 1).getId());
        if(getAlgoChoice() != ALGO_PIXELSORT)
            findViewById(R.id.spinSelectSortMode).setVisibility(View.GONE);

        rg.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup radioGroup, int i) {

                RadioButton b = (RadioButton) findViewById(i);
                int algo_index = radioGroup.indexOfChild(b) + 1;
                if(isUnary(getAlgoChoice()) && !isUnary(algo_index) ) {
                    ((Button)findViewById(R.id.btnDataSourceSelection)).setVisibility(View.VISIBLE);
                } else if(!isUnary(getAlgoChoice()) && isUnary(algo_index)) {
                    ((Button)findViewById(R.id.btnDataSourceSelection)).setVisibility(View.GONE);
                }

                if(algo_index == ALGO_PIXELSORT && getAlgoChoice() != ALGO_PIXELSORT)
                    findViewById(R.id.spinSelectSortMode).setVisibility(View.VISIBLE);
                if(algo_index != ALGO_PIXELSORT && getAlgoChoice() == ALGO_PIXELSORT)
                    findViewById(R.id.spinSelectSortMode).setVisibility(View.GONE);
                setAlgoChoice(algo_index);
            }
        });

        Button btnSelectDataSource = (Button)findViewById(R.id.btnDataSourceSelection);
        btnSelectDataSource.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(BendOptionsActivity.this, DataSourceSelectionActivity.class));
            }
        });
        final ToggleButton dmodeButton = (ToggleButton) findViewById(R.id.toggleDatamode);
        if(GlitchAlgo.getDatamode() == 1) {
            dmodeButton.setChecked(GlitchAlgo.getDatamode() == 1);
        } else {
            for(int i = 0; i <showns.length; ++i)
                hideAlgo(showns[i]);
        }
        /*final Spinner forceFmtSpinner = (Spinner)findViewById(R.id.spinForceFormat);
        forceFmtSpinner.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

            }
        });*/
        final Spinner spinSelectSortMode = (Spinner)findViewById(R.id.spinSelectSortMode);
        spinSelectSortMode.setSelection(getSortingMode());
        spinSelectSortMode.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                setSortingMode(position);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        dmodeButton.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                GlitchAlgo.setDatamode(isChecked ? 1 : 0);
                if(isChecked) {
                    MainActivity.createRawFile(0);
                    for(int i = 0; i < hiddens.length; ++i)
                        hideAlgo(hiddens[i]);
                    for(int i = 0; i <showns.length; ++i)
                        showAlgo(showns[i]);
                    displayRawDataWarning(dmodeButton);
                } else {
                    for(int i = 0; i < hiddens.length; ++i)
                        showAlgo(hiddens[i]);

                    for(int i = 0; i <showns.length; ++i)
                        hideAlgo(showns[i]);
                }
            }
        });
    }
    @Override
    public void onBackPressed() {

        GlitchAlgo.startBackgroundTask();
        super.onBackPressed();
    }
}
