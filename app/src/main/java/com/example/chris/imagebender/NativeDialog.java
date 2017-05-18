package com.example.chris.imagebender;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.support.v7.app.AlertDialog;
import android.widget.EditText;

/**
 * Created by chris on 1/19/17.
 */

public class NativeDialog {
    static void makeNativeDlg(Context c, final long onresult, String hint, String title, String pos, String neg, final boolean runOnThread) {
        AlertDialog.Builder builder = new AlertDialog.Builder(c);
        builder.setTitle(title);
        final EditText input = new EditText(c);
        input.setHint(hint);
        builder.setView(input);

        builder.setPositiveButton(pos, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if(runOnThread) {
                    (new Thread() {
                        public void run() {
                            donative(input.getText().toString(), onresult);
                        }
                    }).start();
                } else {
                    donative(input.getText().toString(), onresult);
                }
            }
        });
        builder.setNegativeButton(neg, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                donative(null, onresult);
                dialog.cancel();
            }
        });

        builder.show();
    }

    private static native void donative(String input, long call);

    public static void fatalError(final Activity a, String errmsg) {
        AlertDialog.Builder builder = new AlertDialog.Builder( a );
        builder
                .setMessage( "Error: " + errmsg )
                .setCancelable( false )
                .setNeutralButton( "Ok.", new DialogInterface.OnClickListener()
                {
                    public void onClick ( DialogInterface dialog, int which )
                    {
                        a.finish();
                    }
                } );
        //AlertDialog error = builder.create();
        //error.show();
        builder.show();
        return;
    }
}
