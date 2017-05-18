package com.example.chris.imagebender;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.ImageView;

/**
 * Created by chris on 12/11/16.
 */

public class BendingView extends ImageView/*SurfaceView implements SurfaceHolder.Callback*/ {
    private Bitmap bendBmp;
    private int originH, originW;
    public void setBitmap(Bitmap bmp) {

        bendBmp = bmp;
        this.setImageBitmap(bmp);
    }

    public Bitmap getBitmap() {
        return bendBmp;
    }

    private void initialize() {

    }


    public BendingView(Context context) {
        super(context);
        initialize();
    }

    public BendingView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initialize();
    }

    public BendingView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initialize();
    }

}
