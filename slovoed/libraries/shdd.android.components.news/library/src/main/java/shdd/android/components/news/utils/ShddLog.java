package shdd.android.components.news.utils;

import android.util.Log;

import shdd.android.components.news.AdsManager;
import shdd.android.components.news.Resources;

import java.util.Date;

public class ShddLog {
	
	private static boolean allowLogs = false;
	
	public static void allowLogs ( boolean allow ) {
		if(allowLogs == allow) return;
		allowLogs = allow;
	}
	
	public static int d(String tag, String msg) {
        return allowLogs? Log.e(tag, msg) : 0;
    }
	public static int e(String tag, String msg) {
		return allowLogs? Log.e(tag, msg) : 0;
	}
	public static int i(String tag, String msg) {
		return allowLogs? Log.i(tag, msg) : 0;
	}

    public static void printSchedule(Long triggerAtTime) {
        d("shdd", "adsAgree:" + Resources.getters.isAdsAgree()
                + "\n\r today       :" + new Date()
                + "\n\r last update :" + new Date(AdsManager.getPreferences().getLong(AdsManager.PREFS_KEY_LAST_CHECK_DATE, 0))
                + "\n\r next        :" + (null != triggerAtTime? new Date(triggerAtTime): "CANCELED"));
    }
}
