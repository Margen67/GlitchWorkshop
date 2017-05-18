package com.example.chris.imagebender;

import android.graphics.Bitmap;
import static com.example.chris.imagebender.DataSourceSelection.*;

/**
 * Created by chris on 11/23/16.
 */

public class GlitchAlgo {
    public static final int
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

    //private static int selectedAlgo = 1;
    //private static int selectedDataSource; //= SEL_RNG;


    //private static int constantValue = 0;
    //private static String textValue = null;
    private static NativelyCachedBmp originalDataSrcBmp = null;
    //private static NativelyCachedBmp resizedDataSrcBmp = null;

    public static native void initNative();

    private static native void setAlgo(int algo);
    private static native int getAlgo();

    private static native void setDataSrc(int src);
    private static native int getDataSrc();

    private static native void setConst(int val);
    private static native int getConst();


    private static native void setTextVal(String val);
    private static native String getTextVal();

    private static native void setResizedSrcBmp(NativelyCachedBmp bmp, long ptr);
    private static native NativelyCachedBmp getResizedSrcBmp();

    public static int getAlgoChoice() {
        return getAlgo();
    }

    public static void updateDataSrcBmpDimensions() {
        if(MainActivity.originalBitmap != null && getOriginalDataSrcBmp() != null) {
            Bitmap scaled = Bitmap.createScaledBitmap(getOriginalDataSrcBmp().restoreFromCache(), MainActivity.originalBitmap.getW(), MainActivity.originalBitmap.getH(), false);
            GlitchAlgo.setResizedDataSrcBmp(new NativelyCachedBmp(scaled));
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

    public Bitmap weirdifyImage(Bitmap im, int xormask) {
        try {
            final int w = im.getWidth();
            final int h = im.getHeight();

            int[] pixels = new int[w * h];
            im.getPixels(pixels, 0, w, 0, 0, w, h);

            Bitmap newbmp = Bitmap.createBitmap(w, h, im.getConfig());

            processArray(pixels, xormask, getAlgoChoice(), w , h);
            newbmp.setPixels(pixels, 0, w, 0, 0, w, h);
            return newbmp;
        } catch(Exception e ) {
            final int w = im.getWidth();
            final int h = im.getHeight();

            int[] pixels = new int[w * h];
            im.getPixels(pixels, 0, w, 0, 0, w, h);
            processArray(pixels, xormask, getAlgoChoice(), w, h);
            im.setPixels(pixels, 0, w, 0, 0, w, h);
            return im;
        }
    }

    public void maxoutAlpha(Bitmap im) {
        final int w = im.getWidth();
        final int h = im.getHeight();

        int[] pixels = new int[w * h];
        im.getPixels(pixels, 0, w, 0, 0, w, h);
        maxoutAlphaNative(pixels);
        im.setPixels(pixels, 0, w, 0, 0, w, h);
    }

    /*public static Bitmap randomBitmap(int w, int h) {
        Bitmap bmp = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        int[] pixels = new int[w*h];
        generateRandomArray(pixels);
        bmp.setPixels(pixels, 0, w, 0, 0, w, h);
        return bmp;
    }*/

    public void processCachedBmp(NativelyCachedBmp bmp, int mask) {
        if(getSelectedDataSource() == SEL_CONSTANT || getSelectedDataSource() == SEL_RNG)
            processCachedBmp_(bmp.getPtr(), (getSelectedDataSource() == SEL_CONSTANT) ? getConstantValue() : mask, getAlgoChoice(), getSelectedDataSource());
        else if(getSelectedDataSource() == SEL_IMAGE)
            processCachedBmp_(bmp.getPtr(), getResizedDataSrcBmp().getPtr(), getAlgoChoice(), getSelectedDataSource());
        else if(getSelectedDataSource() == SEL_TEXT)
            processCachedBmp_(bmp.getPtr(), 0, getAlgoChoice(), getSelectedDataSource());
                    //processCachedBmpString(bmp.getPtr(), getTextValue().getBytes(), getAlgoChoice());

    }

    //private static native void generateRandomArray(int[] pixels);

    private native void maxoutAlphaNative(int[] pixels);
    //private native void processCachedBmpString(long ptr, byte[] extraVal, int algo_choice);
    private native void processCachedBmp_(long ptr, long extraVal, int algo_choice, int srcchoice);
    private native void processArray(int[] pixels, int extraVal, int algo_choice, int w, int h);
}
