package com.example.chris.imagebender;

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.provider.MediaStore;

import org.w3c.dom.Node;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;

/**
 * Created by chris on 12/11/16.
 */

public class FileUtils {
    /**
     * Get the value of the data column for this Uri. This is useful for
     * MediaStore Uris, and other file-based ContentProviders.
     *
     * @param context The context.
     * @param uri The Uri to query.
     * @param selection (Optional) Filter used in the query.
     * @param selectionArgs (Optional) Selection arguments used in the query.
     * @return The value of the _data column, which is typically a file path.
     */
    private static String getDataColumn(Context context, Uri uri, String selection, String[] selectionArgs) {
        Cursor cursor = null;
        final String column = MediaStore.Images.Media.DATA;
        final String[] projection = { column };

        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs, null);
            if (cursor != null && cursor.moveToFirst()) {
                final int column_index = cursor.getColumnIndexOrThrow(column);
                return cursor.getString(column_index);
            }
        } catch(IllegalArgumentException e) {
            e.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return null;
    }
    @SuppressWarnings("NewApi")
    public static String readPathFromUri(Context context, Uri uri) {
        final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;

        // DocumentProvider
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            // ExternalStorageProvider
            if (isExternalStorageDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }
            }
            // DownloadsProvider
            else if (isDownloadsDocument(uri)) {
                final String id = DocumentsContract.getDocumentId(uri);
                final Uri contentUri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));

                return getDataColumn(context, contentUri, null, null);
            }
            // MediaProvider
            else if (isMediaDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                Uri contentUri = getContentUri(type);

                final String selection = "_id=?";
                final String[] selectionArgs = new String[] { split[1] };
                return getDataColumn(context, contentUri, selection, selectionArgs);
            }
        }
        // MediaStore (and general)
        else if ("content".equalsIgnoreCase(uri.getScheme())) {
            // Return the remote address
            if (isGooglePhotosUri(uri)) {
                return uri.getLastPathSegment();
            }
            return getDataColumn(context, uri, null, null);
        }
        // File
        else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }

        return null;
    }


    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    private static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is DownloadsProvider.
     */
    private static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is MediaProvider.
     */
    private static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is Google Photos.
     */
    private static boolean isGooglePhotosUri(Uri uri) {
        return "com.google.android.apps.photos.content".equals(uri.getAuthority());
    }


    private static Uri getContentUri(String type) {
        switch (type) {
            case "image":
                return MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
            case "video":
                return MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
            case "audio":
                return MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
        }
        return null;
    }

    public static String getRootDir() {
        return Environment.getExternalStorageDirectory().getAbsolutePath();
    }


    static void clearDirectory(String dir){
        File dirFile = new File(dir);
        File[] flist = dirFile.listFiles();
        for(File file : flist) {
            if(file.isDirectory()) {
                clearDirectory(file.getPath());
                //for (File f2 : file.listFiles())
                 //   f2.delete();
            }
            file.delete();
        }
    }

    static void removeAllSubdirectories(String dir) {
        File dirFile = new File(dir);
        File[] flist = dirFile.listFiles();
        for(File f : flist) {
            if(f.isDirectory()) {
                clearDirectory(f.getPath());
                f.delete();
            }
        }
    }

    static String splitFilenameFromPath(String path) {
        String[] arr = path.split("/");
        return arr[arr.length - 1];
    }

    static boolean hasExtension(File f, String ext){
        String filename = splitFilenameFromPath(f.getPath());
        String realExtension  = "." + ext;
        int extIndex = filename.indexOf(ext);
        if(extIndex == -1 || extIndex + realExtension.length() != filename.length() - 1)
            return false;
        return true;
    }

    static void deleteAllWithExtension(String dir, String extension) {
        File dirFile = new File(dir);
        File[] flist = dirFile.listFiles();
        for(File f:flist) {
            if(hasExtension(f, extension))
                f.delete();
        }
    }

    static void saveBmp(File path, Bitmap savedBmp) {
        try {
            path.createNewFile();
            FileOutputStream out = new FileOutputStream(path);
            savedBmp.compress(Bitmap.CompressFormat.PNG, 100, out);
            out.close();
        }catch(Exception e)
        {
            return;
        }
    }
    static void saveBmp(File path, Bitmap savedBmp, android.graphics.Bitmap.CompressFormat format) {
        try {
            path.createNewFile();
            FileOutputStream out = new FileOutputStream(path);
            savedBmp.compress(format, 100, out);
            out.close();
        }catch(Exception e)
        {
            return;
        }
    }

    public static void copyFile(File sourceFile, File destFile) throws IOException {
        if (!destFile.getParentFile().exists())
            destFile.getParentFile().mkdirs();

        if (!destFile.exists()) {
            destFile.createNewFile();
        }

        FileChannel source = null;
        FileChannel destination = null;

        try {
            source = new FileInputStream(sourceFile).getChannel();
            destination = new FileOutputStream(destFile).getChannel();
            destination.transferFrom(source, 0, source.size());
        } finally {
            if (source != null) {
                source.close();
            }
            if (destination != null) {
                destination.close();
            }
        }
    }
}
