package com.home.frye.webapp;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.os.PowerManager;
import android.app.KeyguardManager;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.text.SimpleDateFormat;
import com.home.frye.webapp.LogToFileUtils;


public class MainActivity extends AppCompatActivity {


    WebView webView;
    //BatteryReceiver BatteryReceiver;

    boolean isLogOutput = false;


    private PowerManager.WakeLock mWakeLock;
    PowerManager mPowerManager;
    private PowerManager.WakeLock mWakeLock2;
    private ComponentName adminReceiver;

    DatagramSocket socket = null;
    InetAddress serverAddress = null;
    private Handler handler = new Handler();
    private Runnable runnable_tx;
    private Runnable runnable_rx;
    int BatteryData[] = {13, 0
            , 0, 0, 0, 0, 0, 0};// data type,BatteryPercentage


    byte[] receiveData;
    DatagramPacket receivePacket;
    byte sendData[];
    DatagramPacket sendPacket;

    String date;
    Boolean isScreenOn = true;
    BatteryReceiver m_receiver;

    String MacAddress;
    String[] Mac;

//    Message msg_rx;
//    Bundle data_rx;

    //used for other activity to call me
    private static MainActivity mainActivity;

    public MainActivity()
    {
        mainActivity = this;
    }

    public static MainActivity getMainActivity()
    {
        return mainActivity;
    }

    public static String getLocalMacAddressFromWifiInfo(Context context) {
        WifiManager wifi = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        WifiInfo winfo = wifi.getConnectionInfo();
        String mac = winfo.getMacAddress();
        return mac;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        LogToFileUtils.init(this);

        //For Screen on and off
        adminReceiver = new ComponentName(MainActivity.this, ScreenOffAdminReceiver.class);
        mPowerManager = (PowerManager) getSystemService(POWER_SERVICE);
        //mWakeLock2 = mPowerManager.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, "tag");
        mWakeLock2 = mPowerManager.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, this.getClass().getCanonicalName());

        //For battery information update
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        m_receiver = new BatteryReceiver();
        registerReceiver(m_receiver, intentFilter);

        //Get MAC address
        MacAddress = getLocalMacAddressFromWifiInfo((Context) this);
        String delimeter = ":";  // 指定分割字符
        Mac = MacAddress.split(delimeter); // 分割字符串

        for (byte i = 0; i < 6; i++) {
            BatteryData[i + 2] = Integer.parseInt(Mac[i], 16);
        }


        //Create UDP socket
        try {
            socket = new DatagramSocket();
            serverAddress = InetAddress.getByName("192.168.0.17");
        } catch (Exception e) {
            e.printStackTrace();
        }

        sendData = new byte[8 + 6 * 4];
        sendPacket = new DatagramPacket(sendData, sendData.length, serverAddress, 5050);
        receiveData = new byte[1024];
        receivePacket = new DatagramPacket(receiveData, receiveData.length);

//        msg_rx = new Message();
//        data_rx = new Bundle();




        runnable_tx = new Runnable() {
            public void run() {
//                SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss"); //获取系系时间
//                date = sDateFormat.format(new java.util.Date());
//                Log.d("Date", "发送时间" + date + " ");


                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        BatteryData[1] = m_receiver.getPercent();

                        for (byte i = 0; i < 8 + 6 * 4; i++) {
                            sendData[i] = (byte) ((BatteryData[i / 4] >> (i % 4 * 8)) & 0xff);
                        }

                        try {
                            LogToFileUtils.write("send 1");
                            socket.send(sendPacket);
                            LogToFileUtils.write("send 2");



                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }).start();
                Message msg = new Message();
                Bundle data = new Bundle();
                msg.setData(data);
                LogToFileUtils.write("send 3");
                handlerTxMsg.sendMessage(msg);
                LogToFileUtils.write("send 4");
                //handler.postDelayed(this, 1000);
            }
        };

        runnable_rx = new Runnable() {
            public void run() {
                //SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss"); //获取系系时间
                //date = sDateFormat.format(new java.util.Date());
                //Log.d("Date", "接收时间" + date + " ");


                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            //Log.d("Date", "try 接收");
                            LogToFileUtils.write("receive 1");
                            socket.receive(receivePacket);
                            LogToFileUtils.write("receive 2");
                            if (receivePacket.getLength() == 4) {
                                //Log.d("Date", "Got "+receivePacket.getLength()+" bytes "+receiveData[0]+" "+receiveData[1]+" "+receiveData[2]+" "+receiveData[3]);
                            }
                            Message msg = new Message();
                            Bundle data = new Bundle();
                            data.putByteArray("value",receiveData);
                            msg.setData(data);
                            LogToFileUtils.write("receive 3");
                            handlerRxMsg.sendMessage(msg);
                            LogToFileUtils.write("receive 4");
//                            data_rx.putByteArray("value",receiveData);
//                            msg_rx.setData(data_rx);
//                            handlerRxMsg.sendMessage(msg_rx);

                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }).start();
            }
        };


        handler.postDelayed(runnable_tx, 1000); // 开始Timerm
        handler.postDelayed(runnable_rx, 1000); // 开始Timerm

        webView = (WebView) findViewById(R.id.webView);
        WebSettings settings = webView.getSettings();
        settings.setJavaScriptEnabled(true);
        //settings.setSupportZoom(true);
        settings.setBuiltInZoomControls(true);
        settings.setUseWideViewPort(true);
        //webView.setInitialScale(150);//12寸平板，竖屏。
        webView.setInitialScale(200);//7寸平板，竖屏。
        //webView.setInitialScale(200);//7寸平板，横屏。
        //webView.setInitialScale(100);//3寸，横屏。
//

        //webView.loadUrl("http://fryefryefrye.myds.me:8084/weather");
        //webView.loadUrl("http://fryefryefrye.myds.me:8084/homeindex");
        webView.loadUrl("http://192.168.0.17:8081/localindex");

    }


    Handler handlerTxMsg = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            Bundle data = msg.getData();
            //String val = data.getString("value");
            //Log.d("Date", "Got "+receiveData[0]+" "+receiveData[1]+" "+receiveData[2]+" "+receiveData[3]);
            //SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss"); //获取系系时间
            //date = sDateFormat.format(new java.util.Date());
            //Log.d("Date", "发送时间" + date + " ");
            handler.postDelayed(runnable_tx, 1000); // 开始Timerm
        }
    };


    Handler handlerRxMsg = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            Bundle data = msg.getData();
            //String val = data.getString("value");
            //Log.d("Date", "Got "+receiveData[0]+" "+receiveData[1]+" "+receiveData[2]+" "+receiveData[3]);
            if ((isScreenOn) && (receiveData[0] == 0)) {
                isScreenOn = false;
                screenOff();
                Log.d("Date", "screenOff()");
                LogToFileUtils.write("screenOff()");
            }

            if ((!isScreenOn) && (receiveData[0] == 1)) {
                isScreenOn = true;
                screenOn();
                Log.d("Date", "screenOn()");
                LogToFileUtils.write("screenOn()");
            }
            handler.postDelayed(runnable_rx, 1); // 开始Timerm
        }
    };

    private void screenOff() {
        DevicePolicyManager policyManager = (DevicePolicyManager) getSystemService(Context.DEVICE_POLICY_SERVICE);
        //ComponentName adminReceiver = new ComponentName(MainActivity.this, ScreenOffAdminReceiver.class);
//        boolean admin = policyManager.isAdminActive(adminReceiver);
//        if (admin) {
            policyManager.lockNow();
//        } else {
//            android.widget.Toast.makeText(this, "没有设备管理权限",
//                    android.widget.Toast.LENGTH_LONG).show();
//        }


        if (mWakeLock == null) {
            PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
                    this.getClass().getCanonicalName());
        }
        if (mWakeLock != null) {
            mWakeLock.acquire();
        }


    }

    public void screenOn() {

        if (mWakeLock != null) {
            mWakeLock.release();
            mWakeLock = null;
        }

        // turn on screen


        //PowerManager mPowerManager = (PowerManager) getSystemService(POWER_SERVICE);
        //PowerManager.WakeLock mWakeLock2 = mPowerManager.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP, "tag");
        mWakeLock2.acquire();
        mWakeLock2.release();


        KeyguardManager keyguardManager = (KeyguardManager) getSystemService(KEYGUARD_SERVICE);

        KeyguardManager.KeyguardLock keyguardLock = keyguardManager.newKeyguardLock("unLock");// 屏幕锁定
        keyguardLock.reenableKeyguard();
        keyguardLock.disableKeyguard(); // 解锁

    }


}
