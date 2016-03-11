package com.tyunmu.hookWechatRandomGame

public class Hook() implements IXposedHookLoadPackage{
	private String hookClassName;
	private String hookMethodName;
	
	public void findHookName (String versionStr) {
		switch(versionStr) {
			case "6.3.5": {
				hookClassName = "com.tencent.mm.sdk.platformtools.ba";
				hookMethodName = "or";
			}
			case "6.3.7": {
				hookClassName = "com.tencent.mm.sdk.platformtools.bc";
				hookMethodName = "oK";
			}
			case "6.3.8": {
				hookClassName = "com.tencent.mm.sdk.platformtools.bc";
				hookMethodName = "oZ"
			}
			case "6.3.9": {
				hookClassName = "com.tencent.mm.sdk.platformtools.bb";
				hookMethodName = "pu";
			}
			case "6.3.11": {
				hookClassName = "com.tencent.mm.sdk.platformtools.ay";
				hookMethodName = "pu"
			}
			default: {
				hookClassName = "";
				hookMethodName = ""
			}
		}
	}
	
	public void handleLoadPackage(LoadPackageParam loadPackageParam) {
		String[] versionNameArray = XposedHelpers.callMethod(XposedHelpers.callStaticMethod(XposedHelpers.findClass("android.app.ActivityThread", null), "currentActivityThread"), 
															"getSystemContext").getPackageManager()
															.getPackageInfo(loadPackageParam.packageName, 0)
															.versionName
															.split("\\.");
		findHookName(versionNameArray);
		if (!hookClassName.empty() & !hookMethodName.empty()) {
			XposedHelpers.findAndHookMethod(hookClassName, classLoder, hookMethod, Integer.class, new XC_MethodHook() {
				@Override
				protected void beforeHookedMethod(MethodHookParam param) {
					int gameType = param.args[0].intValue();
					int dice_val = Integer.parseInt(RandomGame.prefs.getString("mm_dice_num", "0"));  // 骰子
					int morra_val = Integer.parseInt(RandomGame.prefs.getString("mm_morra_num", "0"));  // 猜拳
				switch(gameType) {
					case 2: {
						param.setResult(Integer.valueOf(morra_val));;
					}
					case 5: {
						param.setResult(Integer.valueOf(dice_val));
					}
				}

            return;
			});
		}
	}
}