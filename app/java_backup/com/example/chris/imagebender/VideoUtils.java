package com.example.chris.imagebender;

import com.github.hiteshsondhi88.libffmpeg.ExecuteBinaryResponseHandler;
import com.github.hiteshsondhi88.libffmpeg.FFmpeg;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

/**
 * Created by chris on 12/11/16.
 */

public class VideoUtils {
/*
    public static final int frames_per_second = 12;
    public static final String frame_extension = ".png";
    public static FFmpeg ffmpeg;
    public static boolean ffmpegAvailable = true;
    private volatile static boolean ffmpegSuccess = true;

    public static final int seconds_per_call = 1;

    static boolean ffmpegWait() {
        while(ffmpeg.isFFmpegCommandRunning())
            ;
        return ffmpegSuccess;
    }

    static String convertToAvi(String videopath, String dir) {
        String[] params = {
                "-i",
                videopath,
                "-safe",
                "0",
                "-vcodec",
                "copy",
                "-acodec",
                "copy",
                dir + "temp.avi"
        };
        try {
            ffmpeg.execute(params, new GenericResponseHandler());

        }catch(Exception e) {}
        return dir + "temp.avi";
    }

    static void mergeVideos(String dir, List<String> vidList) {
        File demuxFile = new File(dir + "demux.txt");
        try {
            BufferedWriter out = new BufferedWriter(new FileWriter(demuxFile));
            for(String s : vidList) {
                out.write("file '" + s + "'");
                out.newLine();
            }
            out.flush();
            out.close();

        }catch(IOException e){
            return;
        }
        String[] params = {
                "-f",
                "concat",
                "-safe",
                "0",
                "-i",
                demuxFile.getPath(),
                "-c",
                "copy",
                dir + "result.avi"
        };
        try {
            ffmpeg.execute(params, new GenericResponseHandler());

        }catch(Exception e) {}
    }

    static String merge24Frames(String dir, int index) {
        String resultName = dir + Integer.toString(index) + ".avi";
        String[] params = {
                "-framerate",
                Integer.toString(frames_per_second) + "/1",
                "-i",
                dir + Integer.toString(index) + "/filename%03d" + frame_extension,
                "-c:v",
                "libx264",
                "-r",
                "30",
                "-pix_fmt",
                "yuv420p",
                resultName
        };

        try {
            ffmpeg.execute(params, new GenericResponseHandler());

        }catch(Exception e) {}
        return resultName;
    }


    static void get24Frames(String videopath, String outdir, long offset, final Thread r) {

            int milliseconds = (int) offset % 1000;
            offset /= 1000;
            int seconds = (int) offset % 60;
            offset /= 60;
            int minutes = (int) offset % 60;
            offset /= 60;
            int hours = (int) offset;


            final String outfileName = outdir + "filename%03d" + frame_extension;
            String ss_str = Integer.toString(hours) + ":" + Integer.toString(minutes) + ":" + Integer.toString(seconds);

            if (outdir.charAt(outdir.length() - 1) != '/')
                outdir += "/";

            String[] params = {
                    "-ss",
                    ss_str,
                    "-i",
                    videopath,
                    "-vframes",
                    Integer.toString(frames_per_second*seconds_per_call),
                    "-r",
                    Integer.toString(frames_per_second) + "/1",
                    outfileName
            };

            if (ffmpeg.isFFmpegCommandRunning())
                ffmpeg.killRunningProcesses();
            try {
                ffmpeg.execute(params, new DumpFramesResponseHandler());

        }catch(Exception e) {}
}

    private static class DumpFramesResponseHandler extends ExecuteBinaryResponseHandler {

        public void onStart() {

        }

        public void onProgress(String message) {

        }

        public void onFailure(String message) {
            ffmpegSuccess = false;
        }

        public void onSuccess(String message) {
            ffmpegSuccess = true;
            if (MainActivity.videoLength != -1)
                return;
            final String durationMarker = "Duration: ";
            int durationIndex = message.indexOf(durationMarker);
            int durationEnd = message.indexOf(",", durationIndex);
            String durationString = message.substring(durationIndex + durationMarker.length(), durationEnd);
            String[] increments = durationString.split(":");
            long hours = Integer.parseInt(increments[0]);
            long minutes = Integer.parseInt(increments[1]);
            long seconds = Integer.parseInt(increments[2].charAt(0) + "" + increments[2].charAt(1));

            long milliseconds = 0;

            long result = ((((hours * 60) + minutes) * 60) + seconds) * 1000;

            MainActivity.videoLength = result;


        }
    }

    private static class GenericResponseHandler extends ExecuteBinaryResponseHandler {

        public void onStart() {

        }

        public void onProgress(String message) {

        }

        public void onFailure(String message) {
            ffmpegSuccess = false;
        }

        public void onSuccess(String message) {
            ffmpegSuccess = true;
        }
    }*/
}