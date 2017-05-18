package com.example.chris.imagebender;

import android.graphics.Bitmap;

import java.io.File;

/**
 * Created by chris on 12/12/16.
 */

public class ImageUtils {
    //the returned bitmap is the same reference as the bitmap passed to the function
    //WRONG

    static Bitmap resizeBmp(Bitmap image, int maxWidth, int maxHeight) {
        if(maxHeight <= 0 || maxWidth <= 0)
            return image;
        int width = image.getWidth();
        int height = image.getHeight();
        float ratioBitmap = (float) width / (float) height;
        float ratioMax = (float) maxWidth / (float) maxHeight;

        int finalWidth = maxWidth;
        int finalHeight = maxHeight;
        if (ratioMax > 1) {
            finalWidth = (int) ((float) maxHeight * ratioBitmap);
        } else {
            finalHeight = (int) ((float) maxWidth / ratioBitmap);
        }
        return Bitmap.createScaledBitmap(image, finalWidth, finalHeight, true);

    }
    static File cacheBmp(Bitmap bmp, String name){
        if(bmp == null )
            return null;
        File cacheFile = new File(CSApp.cacheDir, name);
        FileUtils.saveBmp(cacheFile, bmp);
        bmp.recycle();
        bmp = null;
        return cacheFile;
    }
}
