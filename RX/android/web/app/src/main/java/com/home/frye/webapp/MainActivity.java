package com.home.frye.webapp;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.WindowManager;
import android.webkit.WebSettings;
import android.webkit.WebView;

public class MainActivity extends AppCompatActivity {

     WebView webView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        webView = (WebView) findViewById(R.id.webView);
        WebSettings settings = webView.getSettings();
        settings.setJavaScriptEnabled(true);
        //settings.setSupportZoom(true);
        settings.setBuiltInZoomControls(true);
        settings.setUseWideViewPort(true);
        //webView.setInitialScale(150);//12寸平板，竖屏。
        webView.setInitialScale(200);//7寸平板，横屏。
        //webView.setInitialScale(100);//3寸，横屏。
//

        //webView.loadUrl("http://fryefryefrye.myds.me:8084/weather");
        webView.loadUrl("http://fryefryefrye.myds.me:8084/homeindex");
        //webView.loadUrl("http://fryefryefrye.myds.me:8084/localindex");

    }
}
