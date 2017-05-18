package com.example.chris.imagebender;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;

public class BendOptionsActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bend_options);

        RadioGroup rg = (RadioGroup) findViewById(R.id.groupAlgoChoices);
        rg.check(rg.getChildAt(MainActivity.getAlgoObject().getAlgoChoice() - 1).getId());
        rg.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup radioGroup, int i) {

                RadioButton b = (RadioButton) findViewById(i);
                int algo_index = radioGroup.indexOfChild(b) + 1;
                MainActivity.getAlgoObject().setAlgoChoice(algo_index);
            }
        });

        Button btnSelectDataSource = (Button)findViewById(R.id.btnDataSourceSelection);
        btnSelectDataSource.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(BendOptionsActivity.this, DataSourceSelectionActivity.class));
            }
        });
    }
}
