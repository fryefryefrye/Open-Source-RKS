package com.home.frye.beacon;

import android.annotation.TargetApi;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseSettings;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;

import org.altbeacon.beacon.Beacon;
import org.altbeacon.beacon.BeaconParser;
import org.altbeacon.beacon.BeaconTransmitter;

import java.text.SimpleDateFormat;
import java.util.Arrays;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    private Beacon beacon;
    private BluetoothManager btManager;
    private BluetoothAdapter btAdapter;
    private BeaconTransmitter beaconTransmitter;

    private Handler handler = new Handler();
    private Runnable runnable;

    String date;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.d("Beacon","before setupBeacon");
        setupBeacon();
        Log.d("Beacon","after setupBeacon");

        transmitIBeacon();

        Log.d("Beacon","after transmitIBeacon");






        runnable = new Runnable() {
            public void run() {

                SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss"); //获取系系时间
                date = sDateFormat.format(new java.util.Date());
                Log.d("Beacon", "发送时间" + date + " "  );
                //Log.d("Beacon", "transmitIBeacon");
                //transmitIBeacon();
                handler.postDelayed(runnable, 1000);

//                new Thread(new Runnable() {
//                    @Override
//                    public void run() {
//                        Log.d("Beacon", "发送时间" + date + " "  );
//                //Log.d("Beacon","transmitIBeacon");
//                //transmitIBeacon();
//                handler.postDelayed(runnable, 1000);
//                    }
//                }).start();

            }
        };

        handler.postDelayed(runnable, 1000);

    }

    private void setupBeacon() {
        beacon = new Beacon.Builder()
                .setId1("FDA50693A4E24FB1AFCFC6EB07647820") // UUID for beacon
                .setId2("0001") // Major for beacon
                .setId3("0002") // Minor for beacon
                .setManufacturer(0x0118) // Radius Networks.0x0118  Change this for other beacon layouts//0x004C for iPhone
                .setTxPower(-90) // Power in dB
                .setDataFields(Arrays.asList(new Long[]{0l})) // Remove this for beacon layouts without d: fields
                .build();

        btManager = (BluetoothManager)getSystemService (Context.BLUETOOTH_SERVICE);
        btAdapter = btManager.getAdapter ();
        beaconTransmitter = new BeaconTransmitter(this, new BeaconParser()
                .setBeaconLayout ("m:2-3=0215,i:4-19,i:20-21,i:22-23,p:24-24"));
        beaconTransmitter.setAdvertiseTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_HIGH);
        //beaconTransmitter.setAdvertiseTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_LOW);
        //beaconTransmitter.setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY);
        beaconTransmitter.setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_BALANCED);

    }

    private void transmitIBeacon() {
        boolean isSupported = false;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            isSupported = btAdapter.isMultipleAdvertisementSupported();
            if (isSupported) {

                //Log.d("Beacon", "is support advertistment");
                if (beaconTransmitter.isStarted()) {
                    beaconTransmitter.stopAdvertising();
                    //beaconIv.setAnimation(null);

                } else {
//                    beaconTransmitter.setAdvertiseTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_HIGH);
//                    beaconTransmitter.setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY);
                    beaconTransmitter.startAdvertising(beacon, new AdvertiseCallback() {

                        @Override
                        public void onStartFailure(int errorCode) {
                            Log.d("Beacon", "Advertisement start failed with code: " + errorCode);
                        }

                        @TargetApi(Build.VERSION_CODES.LOLLIPOP)
                        @Override
                        public void onStartSuccess(AdvertiseSettings settingsInEffect) {
                            //Log.d("Beacon", "Advertisement start succeeded." + settingsInEffect.toString());
                        }
                    });
                    //beaconIv.startAnimation(anim);
                }
            } else {
                //UiHelper.showErrorMessage(getActivity(), "Your device is not support leBluetooth.");
                Log.d("Beacon", "Your device is not support leBluetooth.");
            }
        } else {
            //UiHelper.showErrorMessage(getActivity(), "Your device is not support leBluetooth.");
            Log.d("Beacon", "Your device is not support leBluetooth.");
        }
    }

//    @Nullable
//    @Override
//    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
//
//        View v = inflater.inflate(R.layout.beacon_simu_fragment, container, false);
//        beaconIv = v.findViewById(R.id.beaconIV);
//        beaconIv.setOnClickListener(this);
//        return v;
//    }
}
