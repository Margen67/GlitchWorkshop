package com.example.chris.imagebender;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.widget.Button;

/**
 * Created by chris on 2/8/17.
 */

public class ToolbarButton extends Button {
    ToolbarButton(Context c) {
        super(c);
    }
    public ToolbarButton(Context context, AttributeSet attrs) {
        super(context, attrs);

    }

    public ToolbarButton(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }
    @Override
    protected void onDraw(Canvas c) {

        /*
        Canvas c2 = new Canvas();
        {

            Bitmap temp = Bitmap.createBitmap(getWidth(), getHeight(), Bitmap.Config.ARGB_8888);
            c2.setBitmap(temp);
            super.onDraw(c2);
            NativelyCachedBmp bmp = new NativelyCachedBmp(temp);
            c2.setBitmap(null);
            temp.recycle();
            temp = null;

            GlitchAlgo.processCachedBmpUnthreaded(bmp.getPtr());
            Bitmap result = bmp.restoreFromCache();
            Rect src = new Rect(0, 0, result.getWidth(), result.getHeight());
            Rect dst = new Rect(getLeft(), getTop(), getRight(), getBottom());
            c.drawBitmap(result, src, dst, null);
            result.recycle();
        }*/
        super.onDraw(c);
        /*
        Rect r = new Rect();
        r.set((int)(getX() - 10.0f), (int)getY(), (int)(getRight() + 10.0f), (int)getBottom());
        Paint p = new Paint();
        p.setStyle(Paint.Style.STROKE);
        p.setStrokeWidth(2.0f);
        p.setColor(0xFF000000);
        c.drawRect(r, p);*/
    }
}
