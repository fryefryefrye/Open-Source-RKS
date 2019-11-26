package com.home.frye.webapp;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.BatteryManager;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.WindowManager;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.widget.Toast;
import android.os.PowerManager;
import android.app.KeyguardManager;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.text.SimpleDateFormat;



public class MainActivity extends AppCompatActivity {


     WebView webView;
    //BatteryReceiver BatteryReceiver;


    private PowerManager.WakeLock mWakeLock;

    DatagramSocket socket = null;
    InetAddress serverAddress = null;
    private Handler handler = new Handler();
    private Runnable runnable;
    int BatteryData[] = {13, 0
                            ,0,0,0,0,0,0};// data type,BatteryPercentage

    String date;
    Boolean isScreenOn = true;
    BatteryReceiver m_receiver;

    String MacAddress;
    String[] Mac;
    public static String getLocalMacAddressFromWifiInfo(Context context){
        WifiManager wifi = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        WifiInfo winfo = wifi.getConnectionInfo();
        String mac =  winfo.getMacAddress();
        return mac;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        //getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        m_receiver = new BatteryReceiver();
        registerReceiver(m_receiver, intentFilter);

        MacAddress = getLocalMacAddressFromWifiInfo((Context)this);
        String delimeter = ":";  // 指定分割字符
        Mac = MacAddress.split(delimeter); // 分割字符串

        for (byte i = 0; i < 6; i++) {
            BatteryData[i+2] = Integer.parseInt(Mac[i],16);
        }


        try {
            socket = new DatagramSocket();
            serverAddress = InetAddress.getByName("192.168.0.17");
        } catch (Exception e) {
            e.printStackTrace();
        }

        runnable = new Runnable() {
            public void run() {
                SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss"); //获取系系时间
                date = sDateFormat.format(new java.util.Date());
                Log.d("Date", "发送时间" + date + " "  );

                new Thread(new Runnable() {
                    @Override
                    public void run() {


                        BatteryData[1] = m_receiver.getPercent();
                        byte sendData[] = new byte[8+6*4];

                        for (byte i = 0; i < 8+6*4; i++) {
                            sendData[i] = (byte) ((BatteryData[i / 4] >> (i % 4 * 8)) & 0xff);
                        }

                        try {
                            DatagramPacket packet = new DatagramPacket(sendData, sendData.length, serverAddress, 5050);
                            socket.send(packet);

                            byte[] receive = new byte[1024];
                            DatagramPacket receivePacket = new DatagramPacket(receive, receive.length);
//                            while (true) {
                               socket.receive(receivePacket);
                                //String result = new String(receivePacket.getData(), 0, receivePacket.getLength(), "utf-8");
                                if (receivePacket.getLength() == 4)
                                {
                                    //Log.d("Date", "Got "+receivePacket.getLength()+" bytes "+receive[0]+" "+receive[1]+" "+receive[2]+" "+receive[3]);

                                    if((isScreenOn) && (receive[0] == 0))
                                    {
                                        isScreenOn = false;
                                        screenOff();
                                        Log.d("Date", "screenOff()");

                                    }

                                     if((!isScreenOn) && (receive[0] == 1))
                                    {
                                        isScreenOn = true;
                                        screenOn();
                                        Log.d("Date", "screenOn()");
                                    }
                                }
//                                if (mWakeLock == null) {
//                                    PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
//                                    mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
//                                            this.getClass().getCanonicalName());
//                                    mWakeLock.acquire();
//                                    mWakeLock.release();
//                                }


                                //}
//                            }

                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }).start();


//                if (mWakeLock == null) {
//                    PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
//                    mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
//                            this.getClass().getCanonicalName());
//                }
//                if (mWakeLock != null) {
//                    mWakeLock.acquire();
//                }


                handler.postDelayed(this, 1000);


//                if (mWakeLock != null) {
//                    mWakeLock.release();
//                    mWakeLock = null;
//                }

            }
        };


        handler.postDelayed(runnable, 1000); // 开始Timerm


        webView = (WebView) findViewById(R.id.webView);
        WebSettings settings = webView.getSettings();
        settings.setJavaScriptEnabled(true);
        //settings.setSupportZoom(true);
        settings.setBuiltInZoomControls(true);
        settings.setUseWideViewPort(true);
        webView.setInitialScale(150);//12寸平板，竖屏。
        //webView.setInitialScale(200);//7寸平板，横屏。
        //webView.setInitialScale(100);//3寸，横屏。
//

        //webView.loadUrl("http://fryefryefrye.myds.me:8084/weather");
        //webView.loadUrl("http://fryefryefrye.myds.me:8084/homeindex");
        webView.loadUrl("http://fryefryefrye.myds.me:8084/localindex");

    }

        private void screenOff()
        {
            DevicePolicyManager policyManager = (DevicePolicyManager) getSystemService(Context.DEVICE_POLICY_SERVICE);
            ComponentName adminReceiver = new ComponentName(MainActivity.this, ScreenOffAdminReceiver.class);
            boolean admin = policyManager.isAdminActive(adminReceiver);
            if (admin)
            {
                policyManager.lockNow();
            }
            else
             {
                android.widget.Toast.makeText(this,"没有设备管理权限",
                        android.widget.Toast.LENGTH_LONG).show();
            }


            if(mWakeLock == null) {
                PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
                mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
                        this.getClass().getCanonicalName());
            }
            if(mWakeLock != null) {
                mWakeLock.acquire();
            }


        }

        public void screenOn()
         {

             if(mWakeLock != null) {
                 mWakeLock.release();
                 mWakeLock = null;
             }

             // turn on screen




        PowerManager mPowerManager = (PowerManager) getSystemService(POWER_SERVICE);
        PowerManager.WakeLock mWakeLock2 = mPowerManager.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, "tag");
        mWakeLock2.acquire();
        mWakeLock2.release();





        KeyguardManager keyguardManager = (KeyguardManager) getSystemService(KEYGUARD_SERVICE);

        KeyguardManager.KeyguardLock keyguardLock = keyguardManager.newKeyguardLock("unLock");// 屏幕锁定
        keyguardLock.reenableKeyguard();
        keyguardLock.disableKeyguard(); // 解锁

         }


}
