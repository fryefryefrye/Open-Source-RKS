//package com.home.frye.beacon.simulator;
//
//
//import android.annotation.TargetApi;
//import android.app.admin.DevicePolicyManager;
//import android.bluetooth.BluetoothAdapter;
//import android.bluetooth.BluetoothManager;
//import android.bluetooth.le.AdvertiseCallback;
//import android.bluetooth.le.AdvertiseSettings;
//import android.content.Context;
//import android.content.Intent;
//import android.content.pm.PackageManager;
//import android.os.Build;
//import android.os.Bundle;
//import android.util.Log;
//import android.view.animation.Animation;
//import android.widget.ImageView;
//
//import org.altbeacon.beacon.Beacon;
//import org.altbeacon.beacon.BeaconParser;
//import org.altbeacon.beacon.BeaconTransmitter;
//
//import java.util.Arrays;
//
//import androidx.annotation.Nullable;
//
//import static android.app.Activity.RESULT_OK;
//import static androidx.core.content.ContextCompat.getSystemService;
//
////import com.example.srikate.ibeacondemo.R;
////import com.example.srikate.ibeacondemo.utils.UiHelper;
//
///**
// * Created by srikate on 10/4/2017 AD.
// */
//
//public class BeaconSimulatorFragment {//extends Fragment implements View.OnClickListener {
//
//    private Beacon beacon;
//    private BluetoothManager btManager;
//    private BluetoothAdapter btAdapter;
//    private ImageView beaconIv;
//    private static String TAG = "BeaconSimulator";
//    private BeaconTransmitter beaconTransmitter;
//    private Animation anim;
//
//    public static BeaconSimulatorFragment newInstance() {
//        return new BeaconSimulatorFragment();
//    }
//
//    @Override
//    public void onCreate(@Nullable Bundle savedInstanceState) {
//        //super.onCreate(savedInstanceState);
//        setupBeacon();
//        setupAnimation();
//    }
//
//    private void setupAnimation() {
//        //anim = AnimationUtils.loadAnimation(getActivity(), R.anim.shake);
//    }
//
//    private void setupBeacon() {
//        beacon = new Beacon.Builder()
//                .setId1("FDA50693A4E24FB1AFCFC6EB07647825") // UUID for beacon
//                .setId2("F0F0") // Major for beacon
//                .setId3("F0F0") // Minor for beacon
//                .setManufacturer(0x004C) // Radius Networks.0x0118  Change this for other beacon layouts//0x004C for iPhone
//                .setTxPower(-56) // Power in dB
//                .setDataFields(Arrays.asList(new Long[]{0l})) // Remove this for beacon layouts without d: fields
//                .build();
//
//        btManager = (BluetoothManager)getSystemService (Context.BLUETOOTH_SERVICE);
//
//        //DevicePolicyManager policyManager = (DevicePolicyManager) getSystemService(Context.DEVICE_POLICY_SERVICE);
//
//
//
//        btAdapter = btManager.getAdapter ();
//
//        beaconTransmitter = new BeaconTransmitter (getActivity(), new BeaconParser()
//                .setBeaconLayout ("m:2-3=0215,i:4-19,i:20-21,i:22-23,p:24-24"));
//    }
//
////    @Nullable
////    @Override
////    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
////
////        View v = inflater.inflate(R.layout.beacon_simu_fragment, container, false);
////        beaconIv = v.findViewById(R.id.beaconIV);
////        beaconIv.setOnClickListener(this);
////        return v;
////    }
//
//    private boolean isBluetoothLEAvailable() {
//        return btAdapter != null && getActivity().getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE);
//    }
//
//    private boolean getBlueToothOn() {
//        return btAdapter != null && btAdapter.isEnabled();
//    }
//
////    @Override
////    public void onClick(View view) {
////        int id = view.getId();
////
////        if (id == R.id.beaconIV) {
////            onBeaconClicked();
////        }
////    }
//
//    private void onBeaconClicked() {
//        if (getBlueToothOn()) {
//            Log.i(TAG, "isBlueToothOn");
//            transmitIBeacon();
//        } else if (!isBluetoothLEAvailable()) {
//            //UiHelper.showErrorMessage(getActivity(), "Bluetooth not available on your device");
//            Log.d("Beacon", "Bluetooth not available on your device");
//        } else {
//            //Log.i(TAG, "BlueTooth is off");
//            Log.d("Beacon", "BlueTooth is off.Please enable bluetooth before transmit iBeacon.");
////            UiHelper.showInformationMessage(getActivity(), "Enable Bluetooth", "Please enable bluetooth before transmit iBeacon.",
////                    false, new DialogInterface.OnClickListener() {
////                        @Override
////                        public void onClick(DialogInterface dialogInterface, int i) {
////                            if (i == DialogInterface.BUTTON_POSITIVE) {
////                                Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
////                                startActivityForResult(enableIntent, 1);
////                            }
////                        }
////                    });
//        }
//    }
//
//    private void transmitIBeacon() {
//        boolean isSupported = false;
//        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
//            isSupported = btAdapter.isMultipleAdvertisementSupported();
//            if (isSupported) {
//
//                Log.v(TAG, "is support advertistment");
//                if (beaconTransmitter.isStarted()) {
//                    beaconTransmitter.stopAdvertising();
//                    beaconIv.setAnimation(null);
//
//                } else {
//                    beaconTransmitter.startAdvertising(beacon, new AdvertiseCallback() {
//
//                        @Override
//                        public void onStartFailure(int errorCode) {
//                            Log.e(TAG, "Advertisement start failed with code: " + errorCode);
//                        }
//
//                        @TargetApi(Build.VERSION_CODES.LOLLIPOP)
//                        @Override
//                        public void onStartSuccess(AdvertiseSettings settingsInEffect) {
//                            Log.i(TAG, "Advertisement start succeeded." + settingsInEffect.toString());
//                        }
//                    });
//                    beaconIv.startAnimation(anim);
//                }
//            } else {
//                //UiHelper.showErrorMessage(getActivity(), "Your device is not support leBluetooth.");
//                Log.d("Beacon", "Your device is not support leBluetooth.");
//            }
//        } else {
//            //UiHelper.showErrorMessage(getActivity(), "Your device is not support leBluetooth.");
//            Log.d("Beacon", "Your device is not support leBluetooth.");
//        }
//    }
//
//    @Override
//    public void onActivityResult(int requestCode, int resultCode, Intent data) {
//        if (resultCode == RESULT_OK) {
//            transmitIBeacon();
//        } else {
//            Log.e(TAG, "result not ok");
//        }
//    }
//}
