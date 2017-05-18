package com.example.chris.imagebender;

import android.graphics.Bitmap;
import static com.example.chris.imagebender.DataSourceSelection.*;

/**
 * Created by chris on 11/23/16.
 */

public class GlitchAlgo {
    public static final int
            ALGO_NONE = 0,
            ALGO_XOR = 1,
            ALGO_OR = 2,
            ALGO_AND = 3,
            ALGO_ADD = 4,
            ALGO_SUB = 5,
            ALGO_MUL = 6,
            ALGO_DIV = 7,
            ALGO_DIV64 = 8,
            ALGO_REVSUB = 9,
            ALGO_RBIT = 10,
            ALGO_RBYTE = 11,
            ALGO_PIXELSORT = 12;
    public static final int
            FORCE_NONE = 0,
            FORCE_JPEG = 1,
            FORCE_PNG = 2;
    public static final int
            DATAMODE_PIXEL = 0,
            DATAMODE_RAW = 1;
    private static NativelyCachedBmp originalDataSrcBmp = null;
    public static native void startBackgroundTask();
    public static native void initNative();

    private static native void setAlgo(int algo);
    private static native int getAlgo();
    public static native void setDatamode(int mode);
    public static native int getDatamode();

    private static native void setDataSrc(int src);
    private static native int getDataSrc();

    private static native void setConst(int val);
    private static native int getConst();


    private static native void setTextVal(String val);
    private static native String getTextVal();
    public static native int getSortingMode();
    public static native void setSortingMode(int i);

    private static native void setResizedSrcBmp(NativelyCachedBmp bmp, long ptr);
    private static native NativelyCachedBmp getResizedSrcBmp();
    public static native boolean getPreserveTransparency();
    public static native void setPreserveTransparency(boolean b);
    public static int getAlgoChoice() {
        return getAlgo();
    }

    public static void updateDataSrcBmpDimensions() {
        if(MainActivity.originalBitmap != null && getOriginalDataSrcBmp() != null) {
            NativelyCachedBmp scaled = new NativelyCachedBmp(getOriginalDataSrcBmp(), MainActivity.originalBitmap.getW(), MainActivity.originalBitmap.getH());
            GlitchAlgo.setResizedDataSrcBmp(scaled);
        }
    }

    public static void setAlgoChoice(int algo_choice) {
        setAlgo(algo_choice);
    }



    public static int getSelectedDataSource() {
        return getDataSrc();
    }

    public static void setSelectedDataSource(int selectedDataSource) {
        setDataSrc(selectedDataSource);
    }


    public static int getConstantValue() {
        return getConst();
    }

    public static void setConstantValue(int constantValue) {
        setConst(constantValue);
    }

    public static String getTextValue() {
        return getTextVal();
    }

    public static void setTextValue(String textValue) {
        setTextVal(textValue);
    }

    public static NativelyCachedBmp getOriginalDataSrcBmp() {
        return originalDataSrcBmp;
    }

    public static void setOriginalDataSrcBmp(NativelyCachedBmp originalDataSrcBmp) {
        GlitchAlgo.originalDataSrcBmp = originalDataSrcBmp;
    }

    public static NativelyCachedBmp getResizedDataSrcBmp() {
        return getResizedSrcBmp();
    }

    public static void setResizedDataSrcBmp(NativelyCachedBmp resizedDataSrcBmp) {
        setResizedSrcBmp(resizedDataSrcBmp, resizedDataSrcBmp.getPtr());
    }

    public static native void notifyRawFile(String s);


    public void maxoutAlpha(Bitmap im) {
        final int w = im.getWidth();
        final int h = im.getHeight();

        int[] pixels = new int[w * h];
        im.getPixels(pixels, 0, w, 0, 0, w, h);
        maxoutAlphaNative(pixels);
        im.setPixels(pixels, 0, w, 0, 0, w, h);
    }


    private native void maxoutAlphaNative(int[] pixels);
    public native void processCachedBmp(long ptr);
    public static native void processCachedBmpUnthreaded(long ptr);
    public static native boolean undoGlitch(long ptr);
    public static native void clearUndo();
    public static native boolean redoGlitch(long ptr);
    public static native void clearRedo();

}
