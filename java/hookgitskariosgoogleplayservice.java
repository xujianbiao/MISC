package com.tym.arvin.hookgitskariosgoogleplayservice;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.util.Log;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import static de.robv.android.xposed.XposedHelpers.findAndHookMethod;
import static de.robv.android.xposed.XposedHelpers.findClass;

/**
 * Created by arvin on 2016/3/2.
 */
public class Hook implements IXposedHookLoadPackage {

    final String PACKAGE_NAME = "com.alorma.github";
    final String LOG_TAG = "Hook";

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam loadPackageParam) throws Throwable {
        // only care about the git app
        if (!loadPackageParam.packageName.equals(PACKAGE_NAME))
            return;

        Log.e(LOG_TAG, "find the package");

        findAndHookMethod("com.google.android.gms.common.internal.zzg", loadPackageParam.classLoader, "zzc", Context.class, int.class, String.class, new XC_MethodHook() {
            @Override
            protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                RuntimeException runtimeException = new RuntimeException("here");
                runtimeException.fillInStackTrace();
                Log.e(LOG_TAG, "call zzc here", runtimeException);
            }
        });

//        findAndHookConstructor("com.google.android.gms.common.api.internal.zzw$zzb", loadPackageParam.classLoader,
//                findClass("com.google.android.gms.common.api.internal.zzw", loadPackageParam.classLoader),
//                int.class,
//                findClass("com.google.android.gms.common.ConnectionResult", loadPackageParam.classLoader),
//                new XC_MethodHook() {
//                    @Override
//                    protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
//                        Log.e(LOG_TAG, "hook the zzw$zzb");
//                        Log.e(LOG_TAG, "ConnectionResult: " + param.args[1].toString());
//                        param.args[1] = 0;
//                    }
//                }
//        );

        findAndHookMethod("com.google.android.gms.common.GooglePlayServicesUtil", loadPackageParam.classLoader,
                "showErrorDialogFragment",
                int.class,
                Activity.class,
                findClass("android.support.v4.app.Fragment", loadPackageParam.classLoader),
                int.class,
                DialogInterface.OnCancelListener.class,
                new XC_MethodHook() {
                    @Override
                    protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                        Log.e(LOG_TAG, "show dialog hooked");
                        param.args[0] = 0;
                    }
                }
        );
    }
}
