package com.example.chris.imagebender;

import android.content.ActivityNotFoundException;
import android.content.ContentValues;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.media.MediaScannerConnection;
import android.net.Uri;

import android.os.Build;
import android.os.Environment;
import android.provider.MediaStore;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import com.github.hiteshsondhi88.libffmpeg.FFmpeg;
import com.github.hiteshsondhi88.libffmpeg.exceptions.FFmpegNotSupportedException;
import com.squareup.picasso.Picasso;
import com.squareup.picasso.Target;

import java.io.File;

public class MainActivity extends ActionBarActivity {

    public static final String videoCacheDir = "/sdcard/glitchCache/";

    static {
        System.loadLibrary("glitch-native");
    }

    public static NativelyCachedBmp originalBitmap;
    public static NativelyCachedBmp unresizedCurrentBitmap;

    private static Bitmap currentBitmap;


    private static GlitchAlgo glitchAlgos;
    public static CSApp csApp;

    public static int imgMaxHeight = 2048, imgMaxWidth = 2048;
    public static final int IMAGE_OPENED_RESULT = 0, VIDEO_OPENED_RESULT = 1;
    private static BendingView ImgDest = null;
    private Uri lastLoaded = null;
    public static boolean hadToScale;

    private static File cachedCurrentFile;
    //public static volatile long videoLength = -1;

    private Target loadingTarget;
    public static Thread videoBendingThread;

    public CSPrefs csPrefs;

    private boolean needVideoSavename = false;
    private Toolbar toolbar;


    public static Bitmap resizeLoadedBitmap(final Bitmap bitmap) {
        int w = bitmap.getWidth(), h = bitmap.getHeight();
        Bitmap result = null;
        if (w > imgMaxWidth || h > imgMaxHeight) {
            hadToScale = true;
            result = ImageUtils.resizeBmp(bitmap,
                    (imgMaxWidth < csApp.screenWidth ? imgMaxWidth : csApp.screenWidth),
                    (imgMaxHeight < csApp.screenHeight ? imgMaxHeight : csApp.screenHeight));

        } else {
            hadToScale = false;
            result = bitmap;
        }
        return result;
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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        registerToNative();
        GlitchAlgo.initNative();

        toolbar = (Toolbar)findViewById(R.id.tool_bar);
        toolbar.setTitleTextColor(Color.BLUE);
        setSupportActionBar(toolbar);

        csApp = new CSApp(this);
        csPrefs = new CSPrefs(this);

        CSApp.cacheDir  = getApplication().getCacheDir().getPath() + "/";

        if(imgMaxHeight > csApp.screenHeight) imgMaxHeight = csApp.screenHeight;
        if(imgMaxWidth > csApp.screenWidth) imgMaxWidth = csApp.screenWidth;

        ImgDest = (BendingView)findViewById(R.id.ImgDest);
        glitchAlgos = new GlitchAlgo();
        bendVideo("/storage/emulated/0/DCIM/Camera/20170117_153713.mp4");
        VideoProcessingThread.notifyHaveSavename("/sdcard/DCIM/ii.mp4");

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

    public static void saveCurrentBitmapAs(final MainActivity mainActivity, final String saveFileName) {
        final String savePath = FileUtils.getRootDir() + "/DCIM/Camera/" + saveFileName + ".png";

        final File destPath = new File(savePath);
        if(saveFileName == null)
            return;


        Bitmap unresized = unresizedCurrentBitmap.restoreFromCache();
        FileUtils.saveBmp(destPath, unresized);
        unresized.recycle();

        //Bitmap res = resizeLoadedBitmap(unresized);
        //if(unresized != res)

        //mainActivity.setImg(res);


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
        csApp.threadedToast("Saved " + saveFileName + ".png to /DCIM/Camera" );


    }

    private void askImageSavename() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Save Image");
        final EditText input = new EditText(this);
        input.setHint("Name your image");
        builder.setView(input);

        builder.setPositiveButton("Save", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                (new Thread() {
                    public void run() {
                        saveCurrentBitmapAs(MainActivity.this, input.getText().toString());
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
               // VideoProcessingThread.videoSavename = input.getText().toString();
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



    public void setImg(Bitmap bmp) {
        boolean firstLoad = ImgDest.getBitmap() == null;
        if(bmp == currentBitmap)
        {
            return;
        }

        ImgDest.setBitmap(bmp);

        if(!firstLoad && currentBitmap != null)
            currentBitmap.recycle();
        currentBitmap = bmp;
        if(bmp != null)
            ImgDest.invalidate();
    }
    public void openBendOptionsActivity() {
        startActivity(new Intent(this, BendOptionsActivity.class));
    }
    static final int REQUEST_IMAGE_CAPTURE = 1;

    private void dispatchTakePictureIntent() {
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
            startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
        }
    }
    private void disposeCurrentBmp() {
        if(currentBitmap != null) {
            currentBitmap.recycle();
            currentBitmap = null;
        }
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


    public void onBendingRequested() {
        if(originalBitmap == null)
            return;

        setImg(null);
        glitchAlgos.processCachedBmp(unresizedCurrentBitmap, csApp.getRng().nextInt());
        Bitmap unresized = unresizedCurrentBitmap.restoreFromCache();
        Bitmap res = resizeLoadedBitmap(unresized);
        if(unresized != res)
            unresized.recycle();
        setImg(res);
    }


    private void loadImageFromUri(Uri imageUri) {

        if(loadingTarget == null)
            loadingTarget = new BendingTarget(this);
        lastLoaded = imageUri;
        Picasso.with(this).load(imageUri).into(loadingTarget);

    }

    public void onReloadPressed() {
        if(lastLoaded == null)
            return;
        disposeCurrentBmp();
        Bitmap original = originalBitmap.restoreFromCache();
        unresizedCurrentBitmap = new NativelyCachedBmp(original);
        Bitmap res = resizeLoadedBitmap(original);
        if(res != original)
            original.recycle();
        setImg(res);

    }

    public static void shuffleResources(){

        cachedCurrentFile = ImageUtils.cacheBmp(currentBitmap, "cached_original.png");
        currentBitmap = null;

        System.gc();

    }

    public static void restoreResources() {
        if(cachedCurrentFile != null) {
            currentBitmap = BitmapFactory.decodeFile(cachedCurrentFile.getPath());
            cachedCurrentFile = null;

            ImgDest.setBitmap(currentBitmap);
        }
    }

    public void onLoadVideo() {
        /*if(true)
        {
            showToast("Bending videos has been disabled in this version.");
            return;
        }*/


        /*if (!VideoUtils.ffmpegAvailable) {
            showToast("Cannot load video: FFmpeg is unsupported on this device.");
            return;
        }*/
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
        if(needVideoSavename) {
            needVideoSavename = false;
            askVideoSavename();
        }
    }

    void bendVideo(String path) {

        videoBendingThread = new VideoProcessingThread(path, csApp.getRng());
        videoBendingThread.start();

        needVideoSavename = true;
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent i){
        if(resultCode == -1 && requestCode == IMAGE_OPENED_RESULT)
            loadImageFromUri(lastLoaded = i.getData());
        else if(resultCode == -1 && requestCode == VIDEO_OPENED_RESULT) {
            final String videopath = FileUtils.readPathFromUri(this, i.getData());
            bendVideo(videopath);

        }
        else if (requestCode == REQUEST_IMAGE_CAPTURE && resultCode == RESULT_OK) {
            Bundle extras = i.getExtras();
            Bitmap imageBitmap = (Bitmap) extras.get("data");
            setImg(null);
            MainActivity.originalBitmap = new NativelyCachedBmp(imageBitmap);
            MainActivity.unresizedCurrentBitmap = new NativelyCachedBmp(MainActivity.originalBitmap);
            setImg(imageBitmap);
            GlitchAlgo.updateDataSrcBmpDimensions();
        }


    }
}
