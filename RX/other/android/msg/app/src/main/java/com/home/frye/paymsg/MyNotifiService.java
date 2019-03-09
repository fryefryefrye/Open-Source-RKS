package com.home.frye.paymsg;


import android.annotation.SuppressLint;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;
import android.util.Log;
import android.widget.Toast;

import java.io.BufferedWriter;
import java.text.SimpleDateFormat;


/**
 * Created by HouXiang on 2019/3/9.
 */


@SuppressLint("OverrideAbstract")
public class MyNotifiService extends NotificationListenerService {

    private BufferedWriter bw;
    private SimpleDateFormat sdf;
    private MyHandler handler = new MyHandler();
    private String nMessage;
    private String data;


    Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {

            //String msgString = (String) msg.obj;
            //Toast.makeText(getApplicationContext(), msgString, Toast.LENGTH_LONG).show();
        }
    };

//    @Override
//    public void onCreate() {
//        super.onCreate();
//
//        try {
//            socket = new DatagramSocket(5050);
//            serverAddress = InetAddress.getByName("192.168.0.5");
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
//    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i("KEVIN", "Service is started" + "-----");
        data = intent.getStringExtra("data");
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onNotificationPosted(StatusBarNotification sbn) {
        //        super.onNotificationPosted(sbn);
        try {
            //有些通知不能解析出TEXT内容，这里做个信息能判断
            if (sbn.getNotification().tickerText != null) {
                SharedPreferences sp = getSharedPreferences("msg", MODE_PRIVATE);
                nMessage = sbn.getPackageName();
                nMessage += ":";
                nMessage += sbn.getNotification().tickerText.toString();
                Log.e("KEVIN", "Get Message "+ nMessage);
                sp.edit().putString("getMsg", nMessage).apply();
                Message obtain = Message.obtain();
                obtain.obj = nMessage;
                mHandler.sendMessage(obtain);
                //init();
                if (nMessage.contains(data)) {
                    Message message = handler.obtainMessage();
                    message.what = 1;
                    handler.sendMessage(message);
                    //writeData(sdf.format(new Date(System.currentTimeMillis())) + ":" + nMessage);
                }
            }
        } catch (Exception e) {
            Toast.makeText(MyNotifiService.this, "不可解析的通知", Toast.LENGTH_SHORT).show();
        }

    }

//    private void writeData(String str) {
//        try {
////            bw.newLine();
////            bw.write("NOTE");
//            bw.newLine();
//            bw.write(str);
//            bw.newLine();
////            bw.newLine();
//            bw.close();
//        } catch (IOException e) {
//            e.printStackTrace();
//        }
//    }

//    private void init() {
//        sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
//        try {
//            FileOutputStream fos = new FileOutputStream(newFile(), true);
//            OutputStreamWriter osw = new OutputStreamWriter(fos);
//            bw = new BufferedWriter(osw);
//        } catch (IOException e) {
//            Log.d("KEVIN", "BufferedWriter Initialization error");
//        }
//        Log.d("KEVIN", "Initialization Successful");
//    }

//    private File newFile() {
//        File fileDir = new File(Environment.getExternalStorageDirectory().getPath() + File.separator + "ANotification");
//        fileDir.mkdir();
//        String basePath = Environment.getExternalStorageDirectory() + File.separator + "ANotification" + File.separator + "record.txt";
//        return new File(basePath);
//
//    }


    class MyHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case 1:
//                    Toast.makeText(MyService.this,"Bingo",Toast.LENGTH_SHORT).show();
            }
        }

    }
}
