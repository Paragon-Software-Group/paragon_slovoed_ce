package shdd.android.components.news;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class AdsAlarmReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		Intent service = new Intent(context, AdsIntentService.class).putExtra(AdsIntentService.ALARM_KEY, true);
		AdsIntentService.enqueueWork(context, AdsIntentService.class, AdsManager.JOB_ID, service);
	}

}
