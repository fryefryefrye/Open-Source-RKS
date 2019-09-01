package com.home.frye.webapp;

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

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.text.SimpleDateFormat;



public class MainActivity extends AppCompatActivity {

     WebView webView;
    //BatteryReceiver BatteryReceiver;

    DatagramSocket socket = null;
    InetAddress serverAddress = null;
    private Handler handler = new Handler();
    private Runnable runnable;
    int BatteryData[] = {13, 0
                            ,0,0,0,0,0,0};// data type,BatteryPercentage

    String date;
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

//                +  BatteryReceiver.getBatteryPercent()

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
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }).start();


                handler.postDelayed(this, 1000);

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
}
