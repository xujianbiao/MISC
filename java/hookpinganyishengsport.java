package com.tym.arvin.hookpays;

import java.util.Random;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import static de.robv.android.xposed.XposedHelpers.findClass;

/**
 * Created by arvin on 2016/3/5.
 */
public class Hook implements IXposedHookLoadPackage {
    final String PACKAGE_NAME = "com.pingan.papd";
    final String LOG_TAG = "Hook";

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam loadPackageParam) throws Throwable {
        if (!loadPackageParam.packageName.equals(PACKAGE_NAME))
            return;

//        这是存储step_count的加密数据库操作函数，
//        倒是也可以修改，但是服务器会不认，只能在本机上显示
//        findAndHookMethod("com.pajk.hm.sdk.android.util.SecurePreferencesUtil", loadPackageParam.classLoader, "getStepsCount", Context.class, new XC_MethodHook() {
//            @Override
//            protected void afterHookedMethod(MethodHookParam param) throws Throwable {
//                Log.e(LOG_TAG, "get step count");
//                Log.e(LOG_TAG, "result: " + String.valueOf(param.getResult()));
//                param.setResult(new Random().nextInt(20000) + 50000);
//            }
//
//            @Override
//            protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
//            }
//        });


        XposedBridge.hookAllMethods(findClass("android.hardware.SystemSensorManager$SensorEventQueue",loadPackageParam.classLoader), "dispatchSensorEvent", new XC_MethodHook() {
            @Override
            protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                float x = (float)(((float[])param.args[1])[0]);
                float y = (float)(((float[])param.args[1])[1]);
                float z = (float)(((float[])param.args[1])[2]);
                Random random = new Random();
                ((float[]) param.args[1])[0] = (random.nextFloat()) * 3 - (random.nextFloat()) * 2 ;
                ((float[]) param.args[1])[1] = (random.nextFloat()) * 3 - (random.nextFloat()) * 2 ;
                ((float[]) param.args[1])[2] = (random.nextFloat()) * 6 ;
            }
        });
    }
}
