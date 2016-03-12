package com.tym.arvin.hookicebox;

import android.util.Log;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import static de.robv.android.xposed.XposedHelpers.findAndHookMethod;

/**
 * Created by arvin on 2016/3/12.
 */

public class Hook implements IXposedHookLoadPackage{

    final String PACKAGE_NAME = "com.catchingnow.icebox";
    final String TAG = "HOOK";
    final int MAX_APP_ITEMS = 40;
    final String CLASS_NAME = "com.catchingnow.icebox.model.a.a";
    final String METHOD_NAME = "l";

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam loadPackageParam) throws Throwable {
        if (!loadPackageParam.packageName.equals(PACKAGE_NAME))
            return;

        findAndHookMethod(CLASS_NAME, loadPackageParam.classLoader, METHOD_NAME, new XC_MethodHook() {
            @Override
            protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                Log.e(TAG, "find the method");
                Log.e(TAG, "default result: " + String.valueOf(param.getResult()));
                param.setResult(MAX_APP_ITEMS);
            }
        });

    }
}

