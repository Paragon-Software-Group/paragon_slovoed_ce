package shdd.android.components.news;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

import shdd.android.components.news.utils.NotificationUtils;
import shdd.android.components.news.utils.ShddLog;

public final class AdsManager {

	public static final int JOB_ID = 0x0AD5;

	public enum AdsType {
		BANNER,
		NEWS
	}
	public static final String INTENT_KEY_ADS_TYPE = "ads_type";
	public static final String INTENT_KEY_ADS_ID = "ads_id";
	public static final String INTENT_KEY_ADS_FROM_NOTIFICATION = "ads_from_notification";

	public static final String PREFS = "ads_prefs";
    public static final String PREFS_KEY_FORCE_RE_REGISTER_FLAG = "force_re_register_flag";
    public static final String PREFS_KEY_LAST_CHECK_DATE = "last_check_date";
	static final int ALARM_NEWS_CHECK_ID = 42;
	
	public static final long CHECK_INTERVAL_1_DAY = 24 * 60 * 60 * 1000;

	private static AdsManager _instance;
	private AdsManager() {}

	synchronized
	public static AdsManager getInstance() {
		if (_instance == null) {
			_instance = new AdsManager();
		}
		return _instance;
	}

	public AdsManager init(Context ctx, Resources.Getters getters) {
		Resources.getters = getters;
		ShddLog.allowLogs(Resources.getters.isInTestMode());
		NotificationUtils.createChannels(Resources.getters);
		settingsChanged(ctx);
		return this;
	}


	public AdsManager forceReRegister(Context ctx) {
        ShddLog.d("shdd","Call forceReRegister...");
		ShddLog.allowLogs(Resources.getters.isInTestMode());
		getPreferences().edit().putBoolean(PREFS_KEY_FORCE_RE_REGISTER_FLAG, true).apply();
        settingsChanged(ctx);
		return this;
	}

	public AdsManager settingsChanged(Context ctx) {
		ShddLog.allowLogs(Resources.getters.isInTestMode());
		Intent intent = new Intent(ctx, AdsIntentService.class).putExtra(AdsIntentService.INIT_KEY, true);
		AdsIntentService.enqueueWork(ctx, AdsIntentService.class, JOB_ID, intent);
		return this;
	}

	public AdsManager setNewsReceiver(Runnable receiver) {
		AdsAndNews.getInstance().setNewsReceiver(receiver);
		return this;
	}

	public AdsManager forceUpdateNewsCheckAlarm(Context ctx) {
		Intent intent = new Intent(ctx, AdsIntentService.class).putExtra(AdsIntentService.UPDATE_ALARM_KEY, true);
		AdsIntentService.enqueueWork(ctx, AdsIntentService.class, JOB_ID, intent);
		return this;
	}

	public AdsManager forcePullAdsAndNews(Context ctx) {
		if (Resources.getters.isAllowNetwork() && Resources.getters.isAdsAgree()) {
			Intent intent = new Intent(ctx, AdsIntentService.class).putExtra(AdsIntentService.CHECK_NEWS_KEY, true);
			AdsIntentService.enqueueWork(ctx, AdsIntentService.class, JOB_ID, intent);
		}
		return this;
	}

    public static SharedPreferences getPreferences() {
        return Resources.getters.getAppContext().getSharedPreferences(PREFS, Context.MODE_PRIVATE);
    }
}
