package com.example.chris.imagebender;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
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
    private NativelyCachedBmp bendBmp;
    private Bitmap reoriented;
    private int originH, originW;

    private static native void notifyBitmapChange(long currentUnresized);

    public void setBitmap(NativelyCachedBmp bmp) {

        bendBmp = bmp;
        notifyBitmapChange(MainActivity.unresizedCurrentBitmap.getPtr());
        if(bmp == null) {
            setImageBitmap(null);
            if(reoriented != null)
            {
                reoriented.recycle();
                reoriented = null;
            }

            return;
        }
        if(reoriented != null) //recycle last bitmap
        {
            this.setImageBitmap(null);
            reoriented.recycle();
            reoriented = null;
        }
        {
            Bitmap temp = bmp.restoreFromCache();
            reoriented = ImageUtils.reorientBitmap(temp, bendBmp.getOrientation());
            if(reoriented != temp)
                temp.recycle();
        }
        this.setImageBitmap(reoriented);
    }


    public NativelyCachedBmp getBitmap() {
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
