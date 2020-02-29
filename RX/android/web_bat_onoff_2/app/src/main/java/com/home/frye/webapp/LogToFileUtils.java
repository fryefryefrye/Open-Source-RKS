package com.home.frye.webapp;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.os.Environment;
import android.text.format.Formatter;
import android.util.Log;

import com.home.frye.webapp.MainActivity;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.text.SimpleDateFormat;

public class LogToFileUtils {

    private static Context      mContext;
    private static LogToFileUtils instance;
    private static File         logFile;
    private static SimpleDateFormat logSDF  = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
    private static SimpleDateFormat logName = new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss");
    private static final int              LOG_MAX_SIZE = 10 * 1024 * 1024;
    private static String tag;

    private static final String MY_TAG = "LogToFileUtils";

    public static boolean isDebug(Context context){
        boolean isDebug = context.getApplicationInfo()!=null&&
                (context.getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE)!=0;
        return isDebug;
    }

    public static void init(Context context) {

        //if(isDebug(context))

        if(!MainActivity.getMainActivity().isLogOutput)
        {
            Log.i("Date", "Release version . no log file");
            return;
        }
        Log.i("Date", "Debug version . int log file");

        Log.i(MY_TAG, "init ...");
        if (null == mContext || null == instance || null == logFile || !logFile.exists()) {
            mContext = context;
            instance = new LogToFileUtils();
            logFile = getLogFile();
            Log.i(MY_TAG, "LogFilePath is: " + logFile.getPath());
            long logFileSize = getFileSize(logFile);
            Log.d(MY_TAG, "Log max size is: " + Formatter.formatFileSize(context, LOG_MAX_SIZE));
            Log.i(MY_TAG, "log now size is: " + Formatter.formatFileSize(context, logFileSize));
            if (LOG_MAX_SIZE < logFileSize) {
                resetLogFile();
            }
        } else {
            Log.i(MY_TAG, "LogToFileUtils has been init ...");
        }
    }


    public static void write(Object str) {

        if(!MainActivity.getMainActivity().isLogOutput)
        {
            return;
        }

        if (null == mContext || null == instance || null == logFile || !logFile.exists()) {
            Log.e(MY_TAG, "Initialization failure !!!");
            return;
        }
        Log.i("Date", str.toString());
        String logStr = getFunctionInfo() + " - " + str.toString();
        //Log.i(tag, logStr);

        try {
            BufferedWriter bw = new BufferedWriter(new FileWriter(logFile, true));
            bw.write(logStr);
            bw.write("\r\n");
            bw.flush();
        } catch (Exception e) {
            Log.e(tag, "Write failure !!! " + e.toString());
        }
    }

    private static void resetLogFile() {
        Log.i(MY_TAG, "Reset Log File ... ");
        File lastLogFile = new File(logFile.getParent() + "/lastLog.txt");
        if (lastLogFile.exists()) {
            lastLogFile.delete();
        }
        logFile.renameTo(lastLogFile);
        try {
            logFile.createNewFile();
        } catch (Exception e) {
            Log.e(MY_TAG, "Create log file failure !!! " + e.toString());
        }
    }

    private static long getFileSize(File file) {
        long size = 0;
        if (file.exists()) {
            try {
                FileInputStream fis = new FileInputStream(file);
                size = fis.available();
            } catch (Exception e) {
                Log.e(MY_TAG, e.toString());
            }
        }
        return size;
    }

    private static File getLogFile() {
        File file;

        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            file = new File(Environment.getExternalStorageDirectory().getPath() + "/fryelog/");
        } else {
            file = new File(mContext.getFilesDir().getPath() + "/Log/");
        }

        if (!file.exists()) {
            file.mkdir();
        }

        //File logFile = new File(file.getPath() + "/logs.txt");
        File logFile = new File(file.getPath() + "/"+logName.format(new java.util.Date())+".txt");
        if (!logFile.exists()) {
            try {
                logFile.createNewFile();
            } catch (Exception e) {
                Log.e(MY_TAG, "Create log file failure !!! " + e.toString());
            }
        }
        return logFile;
    }

    private static String getFunctionInfo() {
        StackTraceElement[] sts = Thread.currentThread().getStackTrace();
        if (sts == null) {
            return null;
        }
        for (StackTraceElement st : sts) {
            if (st.isNativeMethod()) {
                continue;
            }
            if (st.getClassName().equals(Thread.class.getName())) {
                continue;
            }
            if (st.getClassName().equals(instance.getClass().getName())) {
                continue;
            }
            tag = st.getFileName();
            return "[" + logSDF.format(new java.util.Date()) + " " + st.getClassName() + " " + st
                    .getMethodName() + " Line:" + st.getLineNumber() + "]";
        }
        return null;
    }

}
