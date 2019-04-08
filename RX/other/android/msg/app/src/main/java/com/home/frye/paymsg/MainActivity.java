package com.home.frye.paymsg;


import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Handler;
import android.provider.Settings;
import android.support.annotation.RequiresApi;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.text.SimpleDateFormat;


public class MainActivity extends AppCompatActivity {

    private EditText editText;

    DatagramSocket socket = null;
    InetAddress serverAddress = null;

    private Handler handler = new Handler();
    private Runnable runnable;

    String date;

    int PayData[] = {6, 0, 0};// data type, pay type, pay amount


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Button button = findViewById(R.id.button);
        Button button2 = findViewById(R.id.button2);
        Button button3 = findViewById(R.id.button3);
        editText = findViewById(R.id.editText);

        Intent intent = new Intent(MainActivity.this, MyNotifiService.class);//启动服务
        startService(intent);//启动服务
        final SharedPreferences sp = getSharedPreferences("msg", MODE_PRIVATE);

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
                Log.d("Date", "发送时间" + date);


                new Thread(new Runnable() {
                    @Override
                    public void run() {

                        String getMsg = sp.getString("getMsg", "");
                        if (!TextUtils.isEmpty(getMsg)) {
                            Log.d("Pay", "获取消息:" + getMsg);
                            String[] temp;
                            String delimeter = ":";  // 指定分割字符
                            temp = getMsg.split(delimeter); // 分割字符串

                            Log.d("Pay", "msg from:" + temp[0]);


                            Log.d("Pay", "字符串用：分割出"+temp.length+"个");
                            if(temp.length == 3) {
                                if (temp[0].equals("com.tencent.mm")) {

                                    Log.d("Pay", "确认微信消息");

                                    int amountStart = temp[2].indexOf("到账");
                                    int amountStop = temp[2].indexOf("元");
                                    int amountDot = temp[2].indexOf(".");
                                    if ((amountStart != -1) && (amountStop != -1) && (amountDot != -1)) {
                                        Log.d("Pay", "确认支付消息");
                                        String Amount = temp[2].substring(amountStart + 2, amountDot) + temp[2].substring(amountDot + 1, amountStop);
                                        Log.d("Pay", "支付金额---" + Amount + "---元");
                                        PayData[1] = 1;
                                        PayData[2] = Integer.parseInt(Amount);
                                    }
                                }
                            }
                            if(temp.length == 2) {
                                if (temp[0].equals("com.eg.android.AlipayGphone")) {

                                    Log.d("Pay", "确认alipay消息");
                                    int amountStart = temp[1].indexOf("收款");
                                    int amountStop = temp[1].indexOf("元");
                                    int amountDot = temp[1].indexOf(".");
                                    if ((amountStart != -1) && (amountStop != -1) && (amountDot != -1)) {
                                        Log.d("Pay", "确认支付消息");
                                        //String Amount =  temp[1].substring(amountStart+2,amountStop);
                                        String Amount = temp[1].substring(amountStart + 2, amountDot) + temp[1].substring(amountDot + 1, amountStop);
                                        Log.d("Pay", "支付金额---" + Amount + "---元");
                                        PayData[1] = 0;
                                        PayData[2] = Integer.parseInt(Amount);
                                    }
                                }
                            }

                            sp.edit().putString("getMsg", "").apply();

                        }

                        byte sendData[] = new byte[12];

                        for (byte i = 0; i < 12; i++) {
                            sendData[i] = (byte) ((PayData[i / 4] >> (i % 4 * 8)) & 0xff);
                        }

                        PayData[2] = 0;


                        try {
                            DatagramPacket packet = new DatagramPacket(sendData, sendData.length, serverAddress, 5050);
                            socket.send(packet);
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }).start();


                handler.postDelayed(this, 1000);
                //postDelayed(this,2000)方法安排一个Runnable对象到主线程队列中
            }
        };

        handler.postDelayed(runnable, 1000); // 开始Timer

        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String getMsg = sp.getString("getMsg", "");
                if (!TextUtils.isEmpty(getMsg)) {
                    editText.setText(getMsg);
                }
            }
        });
        button2.setOnClickListener(new View.OnClickListener() {
            @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP_MR1)
            @Override
            public void onClick(View v) {
                //打开监听引用消息Notification access
                Intent intent_s = new Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS);
                startActivity(intent_s);
            }
        });
        button3.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                Intent intent_p = new Intent(Settings.ACTION_APPLICATION_SETTINGS);
//                startActivity(intent_p);

                System.exit(0);//正常退出App
            }
        });
    }
}
