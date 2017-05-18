package com.example.chris.imagebender;

import android.graphics.Bitmap;

/**
 * Created by chris on 12/13/16.
 */

public class NativelyCachedBmp {
    private int w;
    private int h;
    private long ptr;
    private Bitmap.Config cfg;

    NativelyCachedBmp(Bitmap b) {
        w = b.getWidth();
        h = b.getHeight();
        cfg = b.getConfig();
        int[] pixels = new int[getW() * getH()];
        b.getPixels(pixels, 0, getW(), 0, 0, getW(), getH());
        ptr = cache_bmp(pixels, getW(), getH());
    }
    NativelyCachedBmp(int w, int h, long ptr, Bitmap.Config cfg) {
        this.w = w; this.h = h; this.ptr = ptr; this.cfg = cfg;
    }
    NativelyCachedBmp(NativelyCachedBmp other) {
        w = other.getW();
        h = other.getH();
        cfg = other.getCfg();
        ptr = duplicate_cached_bmp(other.getPtr());
    }
    NativelyCachedBmp(NativelyCachedBmp other, int neww, int newh) {
        w = neww;
        h = newh;
        cfg = other.getCfg();
        ptr = rescale(other.getPtr(), neww, newh);
    }
    NativelyCachedBmp(String filepath) {
        ptr = createFromPath(filepath);
        cfg = Bitmap.Config.ARGB_8888;
    }

    public Bitmap restoreFromCache() {
        int[] pixels = new int[getW() * getH()];
        uncache_bmp(getPtr(), pixels);
        Bitmap result = Bitmap.createBitmap(getW(), getH(), getCfg());
        result.setPixels(pixels, 0, getW(), 0, 0, getW(), getH());
        return result;
    }

    public int[] debugGetPixels() {
        int[] pixels = new int[getW() * getH()];
        uncache_bmp(getPtr(), pixels);
        return pixels;
    }
    public int getOrientation() {
        return get_orientation(ptr);
    }

    private static native int get_orientation(long ptr);
    private native long cache_bmp(int[] pixels, int w, int h);
    private native void uncache_bmp(long ptr, int[] pixels);
    private native void destroy_cached_bmp(long ptr);
    private native long createFromPath(String name);
    private native long rescale(long ptr, int neww, int newh);
    private native long duplicate_cached_bmp(long ptr);
    public static native int nativeW(long ptr);
    public static native int nativeH(long ptr);
    @Override
    protected void finalize() {
        if(getPtr() != 0L) {
            destroy_cached_bmp(getPtr());
            ptr = 0L;
        }
    }

    public long getPtr() {
        return ptr;
    }

    public int getW() {
        return w;
    }

    public int getH() {
        return h;
    }

    public Bitmap.Config getCfg() {
        return cfg;
    }
}
