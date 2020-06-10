package shdd.android.components.news;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.JobIntentService;

import com.google.firebase.messaging.RemoteMessage;

public class AdsIntentService extends JobIntentService {
    static final String ALARM_KEY = "alarm";
    static final String PUSH_KEY = "push";
    static final String PUSH_REMOTE_MESSAGE = "push_remote_message";
    static final String INIT_KEY = "init";
    static final String UPDATE_ALARM_KEY = "update_alarm";
    static final String CHECK_NEWS_KEY = "check_news";
    @Deprecated
    static final String PUSH_INTENT = "push_intent";


    @Override
    protected void onHandleWork(@NonNull Intent intent) {
        if (intent.getBooleanExtra(ALARM_KEY, false)) {
            AdsAndNews.getInstance(this).alarm();
        } else if (intent.getBooleanExtra(PUSH_KEY, false)) {
            RemoteMessage data = intent.getParcelableExtra(PUSH_REMOTE_MESSAGE);
            AdsAndNews.getInstance(this).push(data);
        } else if (intent.getBooleanExtra(INIT_KEY, false)) {
            AdsAndNews.getInstance(this).processImpl();
        } else if (intent.getBooleanExtra(UPDATE_ALARM_KEY, false)) {
            AdsAndNews.getInstance(this).forceUpdateNewsCheckAlarm();
        } else if (intent.getBooleanExtra(CHECK_NEWS_KEY, false)) {
            AdsAndNews.getInstance(this).checkAdsAndNews(new Bundle());
        }
    }
}
