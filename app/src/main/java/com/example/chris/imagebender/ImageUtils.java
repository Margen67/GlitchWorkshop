package com.example.chris.imagebender;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;

import java.io.File;

/**
 * Created by chris on 12/12/16.
 */

public class ImageUtils {
    public static Bitmap createBitmap(int[] test, Bitmap source, int x, int y, int width, int height,
                                      Matrix m, boolean filter) {
        if (x + width > source.getWidth()) {
            throw new IllegalArgumentException("x + width must be <= bitmap.width()");
        }
        if (y + height > source.getHeight()) {
            throw new IllegalArgumentException("y + height must be <= bitmap.height()");
        }

        // check if we can just return our argument unchanged
        if (!source.isMutable() && x == 0 && y == 0 && width == source.getWidth() &&
                height == source.getHeight() && (m == null || m.isIdentity())) {
            return source;
        }

        int neww = width;
        int newh = height;
        Canvas canvas = new Canvas();
        Bitmap bitmap;
        Paint paint;

        Rect srcR = new Rect(x, y, x + width, y + height);
        RectF dstR = new RectF(0, 0, width, height);

        Bitmap.Config newConfig = Bitmap.Config.ARGB_8888;
        final Bitmap.Config config = source.getConfig();
        // GIF files generate null configs, assume ARGB_8888
        if (config != null) {
            switch (config) {
                case RGB_565:
                    newConfig = Bitmap.Config.RGB_565;
                    break;
                case ALPHA_8:
                    newConfig = Bitmap.Config.ALPHA_8;
                    break;
                //noinspection deprecation
                case ARGB_4444:
                case ARGB_8888:
                default:
                    newConfig = Bitmap.Config.ARGB_8888;
                    break;
            }
        }

        if (m == null || m.isIdentity()) {
            bitmap = Bitmap.createBitmap(neww, newh, newConfig);//Bitmap.createBitmap(neww, newh, newConfig, source.hasAlpha());
            paint = null;   // not needed
        } else {
            final boolean transformed = !m.rectStaysRect();

            RectF deviceR = new RectF();
            m.mapRect(deviceR, dstR);

            neww = Math.round(deviceR.width());
            newh = Math.round(deviceR.height());

            bitmap = Bitmap.createBitmap(neww, newh, (transformed ? Bitmap.Config.ARGB_8888 : newConfig)/*,
                    (transformed || source.hasAlpha())*/);

            canvas.translate(-deviceR.left, -deviceR.top);
            canvas.concat(m);

            paint = new Paint();
            paint.setFilterBitmap(filter);
            if (transformed) {
                paint.setAntiAlias(true);
            }
        }

        // The new bitmap was created from a known bitmap source so assume that
        // they use the same density
        bitmap.setDensity(source.getDensity());
        bitmap.setHasAlpha(source.hasAlpha());
        bitmap.setPremultiplied(true);

        canvas.setBitmap(bitmap);
        //canvas.drawBitmap(test, );
        canvas.drawBitmap(source, srcR, dstR, paint);
        canvas.setBitmap(null);

        return bitmap;
    }
    /*public static Bitmap createScaledBitmapFromArray(int[] src, int width, int height, int dstWidth, int dstHeight,
                                            boolean filter) {
        Matrix m = new Matrix();
        final float sx = dstWidth  / (float)width;
        final float sy = dstHeight / (float)height;
        m.setScale(sx, sy);
        Bitmap b = Bitmap.createBitmap(src, 0, 0, width, height, m, filter);

        return b;
    }*/
    public static Bitmap rotateBmp(Bitmap b, float value) {
        Matrix matrix = new Matrix();

        matrix.postRotate(value);
        return Bitmap.createBitmap(b , 0, 0, b.getWidth(), b.getHeight(), matrix, true);
    }
    public static Bitmap prescaleBmp(Bitmap b, float sx, float sy) {
        Matrix matrix = new Matrix();

        matrix.preScale(sx, sy);
        return Bitmap.createBitmap(b , 0, 0, b.getWidth(), b.getHeight(), matrix, true);
    }
    public static Bitmap rotateAndFlip(Bitmap b, float rotation, float sx, float sy) {
        Matrix matrix = new Matrix();
        matrix.postRotate(rotation);
        matrix.preScale(sx, sy);
        return Bitmap.createBitmap(b , 0, 0, b.getWidth(), b.getHeight(), matrix, true);
    }

    public static Bitmap reorientBitmap(Bitmap temp, int orientation) {
        Bitmap reoriented = null;
        switch(orientation) {
            case 1: //no reorienting needed
                reoriented = temp;
                break;
            case 2: //flip horizontal
                reoriented = prescaleBmp(temp, -1, 1);
                break;
            case 3:
                reoriented = rotateBmp(temp, 180);
                break;
            case 4:
                reoriented = prescaleBmp(temp, 1, -1);
                break;
            case 5:
                reoriented = rotateAndFlip(temp, 270, 1, -1);
                break;
            case 6:  //rotate 90
                reoriented = rotateBmp(temp, 90);
                break;
            case 7:
                reoriented = rotateAndFlip(temp, 90, 1, -1);
                break;
            case -1:
                reoriented = temp;
                break;
        }
        return reoriented;
    }
    static Point calculateNewDims(NativelyCachedBmp image, int maxWidth, int maxHeight) {
        int width = image.getW();
        int height = image.getH();
        float ratioBitmap = (float) width / (float) height;
        float ratioMax = (float) maxWidth / (float) maxHeight;

        int finalWidth = maxWidth;
        int finalHeight = maxHeight;
        if (ratioMax > 1) {
            finalWidth = (int) ((float) maxHeight * ratioBitmap);
        } else {
            finalHeight = (int) ((float) maxWidth / ratioBitmap);
        }
        return new Point(finalWidth, finalHeight);
    }

    static Bitmap resizeBmp(Bitmap image, int maxWidth, int maxHeight) {
        if (maxHeight <= 0 || maxWidth <= 0)
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
    /*
    static File cacheBmp(Bitmap bmp, String name){
        if(bmp == null )
            return null;
        File cacheFile = new File(CSApp.cacheDir, name);
        FileUtils.saveBmp(cacheFile, bmp);
        bmp.recycle();
        bmp = null;
        return cacheFile;
    }*/
}
