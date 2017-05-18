package com.example.chris.imagebender;

import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;

import android.graphics.Color;
import android.media.MediaScannerConnection;
import android.net.Uri;

import android.os.Build;
import android.os.Environment;
import android.os.SystemClock;
import android.provider.MediaStore;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.AlertDialog;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;


import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends ActionBarActivity {

    static {
        System.loadLibrary("glitch-native");
    }

    public static NativelyCachedBmp originalBitmap;
    public static NativelyCachedBmp unresizedCurrentBitmap;

    private static NativelyCachedBmp currentBitmap;


    private static GlitchAlgo glitchAlgos;
    public static CSApp csApp;

    public static int imgMaxHeight = 2048, imgMaxWidth = 2048;
    public static final int IMAGE_OPENED_RESULT = 0, VIDEO_OPENED_RESULT = 1, REQUEST_IMAGE_CAPTURE = 2;
    private static BendingView ImgDest = null;
    //path to last loaded file
    private String lastLoaded = null;
    public static String cacheDir;
    //public static boolean hadToScale;

    public static Thread videoBendingThread;

    public CSPrefs csPrefs;
    public volatile boolean copiedFFmpeg = false;

    private boolean needVideoSavename = false;
    private Toolbar toolbar;

    public static Bitmap.CompressFormat outputFormat = Bitmap.CompressFormat.JPEG;

    public static native long nativeResizeLoadedBitmap(long bitmap);

    public static NativelyCachedBmp resizeLoadedBitmap(final NativelyCachedBmp bitmap) {
        long ptr = nativeResizeLoadedBitmap(bitmap.getPtr());
        return new NativelyCachedBmp(NativelyCachedBmp.nativeW(ptr), NativelyCachedBmp.nativeH(ptr), ptr, Bitmap.Config.ARGB_8888);

    }
    public void showImageChooser() {
        Intent fileSaveIntent = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        try {
            startActivityForResult(Intent.createChooser(fileSaveIntent, "Select an image to bend"), IMAGE_OPENED_RESULT);
        } catch(ActivityNotFoundException e) {
            showToast("Please install a file manager");
        }
    }

    private native void registerToNative();
    private Button getBtn(int id) {
        return (Button)findViewById(id);
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        cacheDir = getApplication().getCacheDir().getPath();
        CSApp.notifyContextChange(this);
        registerToNative();
        GlitchAlgo.initNative();

        toolbar = (Toolbar)findViewById(R.id.tool_bar);
        //toolbar.setTitleTextColor(Color.BLUE);

        setSupportActionBar(toolbar);

        csApp = new CSApp(this);
        csPrefs = new CSPrefs(this);


        if(imgMaxHeight > csApp.screenHeight) imgMaxHeight = csApp.screenHeight;
        if(imgMaxWidth > csApp.screenWidth) imgMaxWidth = csApp.screenWidth;

        ImgDest = (BendingView)findViewById(R.id.ImgDest);
        glitchAlgos = new GlitchAlgo();
        getBtn(R.id.btnOpen).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showImageChooser();
            }
        });
        getBtn(R.id.btnOptions).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                openBendOptionsActivity();
            }
        });
        getBtn(R.id.btnUndo).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //showToast("Undo is currently unimplemented.");
                if(unresizedCurrentBitmap != null) {
                    if(GlitchAlgo.undoGlitch(unresizedCurrentBitmap.getPtr())) {
                        if(GlitchAlgo.getDatamode() == GlitchAlgo.DATAMODE_RAW)
                            createRawFile(0);
                        setImg(resizeLoadedBitmap(unresizedCurrentBitmap) );
                        GlitchAlgo.startBackgroundTask();
                    }
                }
            }
        });
        getBtn(R.id.btnRedo).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(unresizedCurrentBitmap != null) {
                    if(GlitchAlgo.redoGlitch(unresizedCurrentBitmap.getPtr())) {
                        if(GlitchAlgo.getDatamode() == GlitchAlgo.DATAMODE_RAW)
                            createRawFile(0);
                        setImg(resizeLoadedBitmap(unresizedCurrentBitmap) );
                        GlitchAlgo.startBackgroundTask();
                    }

                }
            }
        });
        getBtn(R.id.btnReload).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onReloadPressed();
            }
        });
        getBtn(R.id.btnSave).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onSave();
            }
        });
        getBtn(R.id.btnCamera).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dispatchTakePictureIntent();
            }
        });
        getBtn(R.id.btnPreferences).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                openPreferencesActivity();
            }
        });
        getBtn(R.id.btnHelp).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                openHelpActivity();
            }
        });
        getBtn(R.id.btnAbout).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                openAboutActivity();
            }
        });
        /*bendVideo("/storage/emulated/0/DCIM/Camera/20170117_153713.mp4");
        VideoProcessingThread.notifyHaveSavename("/sdcard/DCIM/ii.mp4");*/
        /*loadImageFromPath("/storage/emulated/0/DCIM/Camera/20170206_122619.jpg");
        onBendingRequested();*/
        final InputStream ffmpeglib = getResources().openRawResource(R.raw.ffmpeg);
        try {
            final FileOutputStream outLib = new FileOutputStream(cacheDir + "/ffmpeg.so");
            Thread copyLibThread = new Thread() {
                public void run() {
                    try {
                        int available = ffmpeglib.available();
                        {
                            byte buff[] = new byte[1000000];
                            while (available >= 1000000) {
                                ffmpeglib.read(buff);
                                available -= 1000000;
                                outLib.write(buff);
                            }
                        }
                        byte rest[] = new byte[available];
                        /*while (available != 0) {
                            outLib.write(ffmpeglib.read());
                            available--;
                        }*/
                        ffmpeglib.read(rest);
                        outLib.write(rest);
                        ffmpeglib.close();
                        outLib.close();
                    }catch (Exception e) {

                    }

                    return;
                }
            };
            copyLibThread.start();
        }catch (Exception e) {

        }




    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menumain, menu);
        return true;
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        switch(id) {
            case R.id.action_load_image:
                showImageChooser();
                break;
            case R.id.action_bend_image:
                onBendingRequested();
                break;
            case R.id.action_reload_image:
                onReloadPressed();
                break;
            case R.id.action_bend_options:
                openBendOptionsActivity();
                break;
            case R.id.action_save_image:
                onSave();
                break;
            case R.id.action_bend_video:
                onLoadVideo();
                break;
            case R.id.action_take_image:
                dispatchTakePictureIntent();
                break;
            case R.id.action_open_preferences:
                openPreferencesActivity();
                break;
            default:
                break;
        }
        return super.onOptionsItemSelected(item);
    }


    public void showToast(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
    }

    public static GlitchAlgo getAlgoObject() {
        return glitchAlgos;
    }
    private static volatile boolean releasedBmp = false;
    public static void saveCurrentBitmapAs(final MainActivity mainActivity, final String saveFileName) {
        String extension = null;
        switch(outputFormat) {
            case JPEG:
                extension = ".jpg";
                break;
            case WEBP:
                extension = ".webp";
                break;
            case PNG:
                extension = ".png";
                break;
        }
        if(extension == null) {
            csApp.threadedToast("Unknown output format!");
            return;
        }
        final String savePath = FileUtils.getRootDir() + "/DCIM/Camera/" + saveFileName + extension;

        final File destPath = new File(savePath);
        if(saveFileName == null)
            return;

        /*mainActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ImgDest.setBitmap(null);
                releasedBmp = true;
            }
        });
        while(!releasedBmp) ;*/
        long start = SystemClock.currentThreadTimeMillis();
        releasedBmp = false;
        {
            Bitmap reoriented;
            {
                Bitmap unresized = unresizedCurrentBitmap.restoreFromCache();
                reoriented = ImageUtils.reorientBitmap(unresized, unresizedCurrentBitmap.getOrientation());
                if (unresized != reoriented)
                    unresized.recycle();
            }
            //FileUtils.saveBmp(destPath, reoriented);
            FileUtils.saveBmp(destPath, reoriented, outputFormat);
            reoriented.recycle();
        }
        /*
        mainActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                MainActivity.setImg(resizeLoadedBitmap(unresizedCurrentBitmap));
            }
        });*/

        csApp.threadedToast("Saved " + saveFileName +
                ".png to /DCIM/Camera. Saving took " + Long.toString(SystemClock.currentThreadTimeMillis() - start)  + " milliseconds.");
        ContentValues values = new ContentValues();

        values.put(MediaStore.Images.Media.DATE_TAKEN, System.currentTimeMillis());
        values.put(MediaStore.Images.Media.MIME_TYPE, "image/png");
        values.put(MediaStore.MediaColumns.DATA, savePath);

        mainActivity.getContentResolver().insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values);
        MediaScannerConnection.scanFile(mainActivity,
                new String[] { destPath.toString() }, null,
                new MediaScannerConnection.OnScanCompletedListener() {
                    public void onScanCompleted(String path, Uri uri) {
                        Log.i("ExternalStorage", "Scanned " + path + ":");
                        Log.i("ExternalStorage", "-> uri=" + uri);
                    }
                });
        mainActivity.refreshAndroidGallery(Uri.parse(savePath));
        mainActivity.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri.fromFile(destPath)));



    }

    private void askImageSavename() {
        if(currentBitmap == null)
        {
            showToast("Load an image before trying to save!");
            return;
        }
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Save Image");
        final EditText input = new EditText(this);
        input.setHint("Name your image");
        builder.setView(input);

        builder.setPositiveButton("Save", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                final ProgressDialog dlg = ProgressDialog.show(MainActivity.this, "Saving image", "Please wait...", true);
                (new Thread() {
                    public void run() {
                        saveCurrentBitmapAs(MainActivity.this, input.getText().toString());
                        dlg.dismiss();
                    }
                }).start();
            }
        });
        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        });

        builder.show();
    }

    private void askVideoSavename() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Save Video");
        final EditText input = new EditText(this);
        input.setHint("Name your video");
        builder.setView(input);

        builder.setPositiveButton("Save", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                VideoProcessingThread.notifyHaveSavename(input.getText().toString());
            }
        });
        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                videoBendingThread.interrupt();
                dialog.cancel();
            }
        });

        builder.show();
    }



    public static void setImg(NativelyCachedBmp bmp) {
        boolean firstLoad = ImgDest.getBitmap() == null;
        if(bmp == currentBitmap)
            return;


        ImgDest.setBitmap(bmp);

        currentBitmap = bmp;
        if(bmp != null)
            ImgDest.invalidate();
    }
    public void openBendOptionsActivity() {
        startActivity(new Intent(this, BendOptionsActivity.class));
    }
    public void openPreferencesActivity() {
        startActivity(new Intent(this, PreferencesActivity.class));
    }
    public void openHelpActivity() {
        startActivity(new Intent(this, HelpActivity.class));
    }
    public void openAboutActivity() {
        startActivity(new Intent(this, AboutActivity.class));
    }

    private void dispatchTakePictureIntent() {
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
            startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
        }
    }
    private void disposeCurrentBmp() {
        if(currentBitmap != null)
            currentBitmap = null;

        ImgDest.setBitmap(null);
    }

    public void onSave() {
        askImageSavename();

    }
    public void refreshAndroidGallery(Uri fileUri) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            Intent mediaScanIntent = new Intent(
                    Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
            mediaScanIntent.setData(fileUri);
            sendBroadcast(mediaScanIntent);
        } else {
            sendBroadcast(new Intent(
                    Intent.ACTION_MEDIA_MOUNTED,
                    Uri.parse("file://" + Environment.getExternalStorageDirectory())));
        }
    }
    public void onBendClick(View v) {
        onBendingRequested();
    }

    public void onBendingRequested() {
        if(originalBitmap == null)
            return;
        if(csApp.getPrefs().getInt("last_algo_selection") == -1) {
            showToast("Please select an algorithm in the options menu before glitching.");
            return;
        }
        long bendStartTime = SystemClock.currentThreadTimeMillis();
        //setImg(null);
        //ProgressDialog dialog = ProgressDialog.show(this, "Processing image", "Please wait...", true);
        glitchAlgos.processCachedBmp(unresizedCurrentBitmap.getPtr());
        setImg(null);
        long after_processing = SystemClock.currentThreadTimeMillis();
        NativelyCachedBmp res = resizeLoadedBitmap(unresizedCurrentBitmap);
        long after_resizing = SystemClock.currentThreadTimeMillis();
        if(GlitchAlgo.getAlgoChoice() != GlitchAlgo.ALGO_PIXELSORT)
            showToast("Databending took " + Long.toString(after_processing - bendStartTime) +
                " milliseconds. With the time taken to resize the image it took a total of " + Long.toString(after_resizing - bendStartTime) + " milliseconds.");
        setImg(res);
        //dialog.dismiss();
        GlitchAlgo.startBackgroundTask();
    }

    public static void createRawFile(int forceFmt) {
        if(rawDisabled)
            return;
        Bitmap current = unresizedCurrentBitmap.restoreFromCache();
        File f = new File(cacheDir + "/tempfile.jpg");
        FileUtils.saveBmp(f, current, Bitmap.CompressFormat.JPEG);
        current.recycle();
        GlitchAlgo.notifyRawFile(f.getPath());
    }
    public static boolean rawDisabled = true;

    private void loadImageFromPath(String imagepath) {
        if(lastLoaded != null) {
            GlitchAlgo.clearUndo();
            GlitchAlgo.clearRedo();
        }
        lastLoaded = imagepath;
        originalBitmap = new NativelyCachedBmp(imagepath);
        unresizedCurrentBitmap = new NativelyCachedBmp(originalBitmap);
        if(!rawDisabled&&GlitchAlgo.getDatamode() == GlitchAlgo.DATAMODE_RAW)
            createRawFile(0);
        setImg(resizeLoadedBitmap(originalBitmap) );
        GlitchAlgo.updateDataSrcBmpDimensions();
        GlitchAlgo.startBackgroundTask();
    }


    private void loadImageFromUri(Uri imageUri) {


        final String imagepath = FileUtils.readPathFromUri(this, imageUri);
        if(imagepath == null) {
            showToast("Couldn't read file path.");
            return;
        }
        loadImageFromPath(imagepath);
    }

    public void onReloadPressed() {
        if(lastLoaded == null)
            return;
        disposeCurrentBmp();
        loadImageFromPath(lastLoaded);

    }

    public void onLoadVideo() {
        if(videoBendingThread != null) {
            showToast("Videobending process is already running.");
            return;
        }
        Intent intent = new Intent("android.intent.action.PICK");
        intent.setType("video/*");
        intent.setAction("android.intent.action.GET_CONTENT");
        startActivityForResult(intent, VIDEO_OPENED_RESULT);
    }
    @Override
    public void onResume() {
        super.onResume();
        CSApp.notifyContextChange((Context) this);
        if(needVideoSavename) {
            needVideoSavename = false;
            askVideoSavename();
        }
    }

    void bendVideo(String path) {

        videoBendingThread = new VideoProcessingThread(path, this);
        videoBendingThread.start();

        needVideoSavename = true;
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent i){
        if(resultCode != RESULT_OK)
            return;

        if(requestCode == IMAGE_OPENED_RESULT)
            loadImageFromUri(i.getData());
        else if(requestCode == VIDEO_OPENED_RESULT) {
            final String videopath = FileUtils.readPathFromUri(this, i.getData());
            /*File b = new File(cacheDir + "/temp.avi");
            if(b.exists())
                b.delete();*/
            bendVideo(videopath);
           // File f = new File(cacheDir + "/temp.avi");
         //   if(f.exists())
       //         f.delete();
        }
        else if (requestCode == REQUEST_IMAGE_CAPTURE) {
            /*
            Bundle extras = i.getExtras();
            {

                Bitmap imageBitmap = (Bitmap) extras.get("data");
                File f = new File(cacheDir + "/tempcamerabmp.png");
                FileUtils.saveBmp(f, imageBitmap);
                imageBitmap.recycle();


            }
            setImg(null);

            MainActivity.originalBitmap = new NativelyCachedBmp(cacheDir + "/tempcamerabmp.png");
            MainActivity.unresizedCurrentBitmap = new NativelyCachedBmp(originalBitmap);
            setImg(resizeLoadedBitmap(originalBitmap));
            GlitchAlgo.updateDataSrcBmpDimensions();
            GlitchAlgo.startBackgroundTask();
            */
            {
                Bitmap b = (Bitmap)(i.getExtras()).get("data");
                FileUtils.saveBmp(new File(cacheDir + "/tempcamerabmp.png"), b);
                b.recycle();

            }
            loadImageFromPath(cacheDir + "/tempcamerabmp.png");
        }


    }

}
