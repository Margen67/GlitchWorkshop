package com.example.chris.imagebender;

import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;

import com.squareup.picasso.Picasso;
import com.squareup.picasso.Target;

/**
 * Created by chris on 12/12/16.
 */
class BendingTarget implements Target {
    private MainActivity mainActivity;

    public BendingTarget(MainActivity mainActivity) {
        this.mainActivity = mainActivity;
    }

    @Override
    public void onBitmapLoaded(Bitmap bitmap, Picasso.LoadedFrom from) {


        MainActivity.originalBitmap = null;

        MainActivity.originalBitmap = new NativelyCachedBmp(bitmap);
        MainActivity.unresizedCurrentBitmap = new NativelyCachedBmp(MainActivity.originalBitmap);

        mainActivity.setImg(MainActivity.resizeLoadedBitmap(bitmap));
        GlitchAlgo.updateDataSrcBmpDimensions();

    }

    @Override
    public void onPrepareLoad(Drawable d) {

    }

    @Override
    public void onBitmapFailed(Drawable d) {

    }
}
