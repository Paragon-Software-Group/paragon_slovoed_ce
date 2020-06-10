package shdd.android.components.news;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;

import com.google.firebase.messaging.RemoteMessage;

import java.lang.ref.WeakReference;
import java.net.URI;
import java.util.Calendar;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.List;

import shdd.android.components.news.data.AdsAd;
import shdd.android.components.news.data.NewsItem;
import shdd.android.components.news.registration.FirebaseCm;
import shdd.android.components.news.registration.ParagonGcmRegisterer;
import shdd.android.components.news.storage.BannerStorage;
import shdd.android.components.news.storage.NewsStorage;
import shdd.android.components.news.utils.NotificationUtils;
import shdd.android.components.news.utils.ShddLog;

class AdsAndNews {
    private static WeakReference<AdsAndNews> _instance;

    private Context serviceContext;
    private WeakReference<Runnable> newsReceiverRef = new WeakReference<Runnable>(null);
    private ParagonGcmRegisterer paragonGcmReg;

    private AdsAndNews() {
        paragonGcmReg = new ParagonGcmRegisterer();
    }

    static synchronized AdsAndNews getInstance() {
        if (_instance == null || _instance.get() == null) {
            _instance = new WeakReference<>(new AdsAndNews());
        }
        return _instance.get();
    }

    static AdsAndNews getInstance(Context serviceContext) {
        getInstance().serviceContext = serviceContext;
        return getInstance();
    }

    void processImpl() {
        checkGCMRegistration();
        initFirstStartCheckDate();
        updateAlarmIfNeed();
    }

    void push(RemoteMessage remoteMessage) {
        if (!Resources.getters.isAllowNetwork()) {
            return;
        }

        checkGCMRegistration();
        if (Resources.getters.isAdsAgree()) {
            Bundle bundle = new Bundle();
            bundle.putBoolean("is_push", true);
            checkAdsAndNews(bundle);
        }
    }


    void alarm() {
        ShddLog.d("shdd", "ALARM FIRE:" + new Date());
        setLastNewsCheckDate(System.currentTimeMillis());
        checkGCMRegistration();
        initFirstStartCheckDate();

        boolean network = Resources.getters.isAllowNetwork();
        if (Resources.getters.isAdsAgree() && network)
            checkAdsAndNews(new Bundle());
        updateAlarmIfNeed();
    }

    void setNewsReceiver(Runnable receiver) {
        newsReceiverRef = new WeakReference<Runnable>(receiver);
    }

    void forceUpdateNewsCheckAlarm() {
        setLastNewsCheckDate(System.currentTimeMillis());
        updateAlarmIfNeed();
    }

    /***
     * @return false - if last check date already defined <br>
     * true - otherwise
     */
    private boolean initFirstStartCheckDate() {
        if (AdsManager.getPreferences().contains(AdsManager.PREFS_KEY_LAST_CHECK_DATE))
            return false;

        setLastNewsCheckDate(System.currentTimeMillis());
        return true;
    }

    private void setLastNewsCheckDate(long milliseconds) {
        AdsManager.getPreferences().edit()
                .putLong(AdsManager.PREFS_KEY_LAST_CHECK_DATE, milliseconds).apply();
    }

    private void updateAlarmIfNeed() {
        AlarmManager am = (AlarmManager) serviceContext
                .getSystemService(Context.ALARM_SERVICE);
        PendingIntent pi = PendingIntent.getBroadcast(serviceContext, AdsManager.ALARM_NEWS_CHECK_ID, new Intent(serviceContext,
                AdsAlarmReceiver.class), PendingIntent.FLAG_UPDATE_CURRENT);

        long triggerAtTime = AdsManager.getPreferences().getLong(AdsManager.PREFS_KEY_LAST_CHECK_DATE, 0) + AdsManager.CHECK_INTERVAL_1_DAY;

        if (Resources.getters.isAdsAgree()) {
            ShddLog.printSchedule(triggerAtTime);
            am.set(AlarmManager.RTC_WAKEUP, triggerAtTime, pi);
        } else {
            ShddLog.printSchedule(null);
            am.cancel(pi);
        }
    }

    synchronized void checkAdsAndNews(Bundle bundle) {
        String type = bundle.getString("type");
        boolean isFromPush = bundle.getBoolean("is_push", false);
        HttpAdsClient client = HttpAdsClient.getAdsClient();
        try {
            if (type == null || "Ad".equals(type))
                checkAds(isFromPush, client);
            if (type == null || "News".equals(type))
                checkNews(isFromPush, client);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void checkAds(boolean isFromPush, HttpAdsClient client) throws Exception {
        if (!Resources.getters.isAllowNetwork()) {
            ShddLog.d("shdd", "Network unavailable");
            return;
        }

        BannerStorage storage = new BannerStorage(serviceContext);
        try {
            BannerStorage.BannerSize bannerSize = new BannerStorage.BannerSize(serviceContext.getResources().getDisplayMetrics());

            HttpAdsClient.Params params = client.getParams();
            params.setFromPush(isFromPush);
            client.pullAdsAndNews(new URI(Resources.ADS_URI), params);
            List<AdsAd> ads = client.getAds();
            ShddLog.d("shdd", "ads obtained from server:" + ads.size());
            for (AdsAd ad : ads) {
                if (ad.getmFinishDate().before(new Date()))
                    continue;

                AdsAd storageAd = storage.getStorageAd(ad, false);
                if (storageAd == null) {
                    if (client.loadImageBytes(ad, bannerSize))
                        storage.insert(ad);

                } else if (BannerStorage.isBannerUrlOrLocaleChanged(storageAd, ad)) {
                    ad.setIsShown(storageAd.isShown());
                    storage.delete(storageAd);
                    if (client.loadImageBytes(ad, bannerSize))
                        storage.insert(ad);

                } else {
                    storage.updateMetadata(ad);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        showNotificationIfActiveAdNotShown(storage);
        int deleted = storage.deleteFinishedAds(new Date().getTime());
        ShddLog.d("shdd", "Ads storage. deleted outdated:" + deleted + " ads in storage:" + storage.getCount());

    }

    private void showNotificationIfActiveAdNotShown(BannerStorage storage) {
        long currentTimeMils = new Date().getTime();
        AdsAd adToNotify = storage.getActiveBannerToNotify(currentTimeMils);
        Runnable newsReceiver = newsReceiverRef.get();
        if (adToNotify != null && newsReceiver == null) {
            ShddLog.d("shdd", "Active ad to notify id:" + adToNotify.getmId());
            NotificationUtils.showNotification(serviceContext, AdsManager.AdsType.BANNER,
                    Resources.getters.getString(Resources.Strings.ADS_NOTIFICATION_BANNER_TITLE), "",
                    Resources.getters.getString(Resources.Strings.ADS_NOTIFICATION_BANNER_TITLE),
                    null);
            storage.updateAdShownState(adToNotify.setIsShown(true));
        }
    }

    private void checkNews(boolean isFromPush, HttpAdsClient client) throws Exception {
        if (!Resources.getters.isAllowNetwork()) {
            ShddLog.d("shdd", "Network unavailable");
            return;
        }

        HttpAdsClient.Params params = client.getParams();
        params.setFromPush(isFromPush);
        client.pullAdsAndNews(new URI(Resources.ADS_URI), params);

        List<NewsItem> newsList = client.getNews();
        if(!newsList.isEmpty()){
            NewsItem news = Collections.max(newsList, new Comparator<NewsItem>() {
                @Override
                public int compare(NewsItem lhs, NewsItem rhs) {
                    int c = lhs.getDate().compareTo(rhs.getDate());
                    return c == 0 ? Integer.valueOf(lhs.getId()).compareTo(rhs.getId()) : c;
                }
            });
            NewsStorage newsStorage = new NewsStorage(serviceContext);
            Runnable newsReceiver = newsReceiverRef.get();
            ShddLog.d("shdd", "news obtained from server:" + newsList.size());
            if (newsStorage.insertNews(newsList)) {
                ShddLog.d("shdd", "news storage has new item. newsReceiver:" + newsReceiver);
                if (newsReceiver != null) {
                    new Handler(Looper.getMainLooper()).post(newsReceiver);
                } else {
                    Calendar c = Calendar.getInstance();
                    c.setTime(news.getDate());
                    c.add(Calendar.DAY_OF_MONTH, 3);
                    if (c.getTimeInMillis() < System.currentTimeMillis())
                        return;
                    Bundle data = new Bundle();
                    data.putInt(AdsManager.INTENT_KEY_ADS_ID, news.getId());
                    String locale = Resources.getters.getString(Resources.Strings.LOCALE);// Utils.getInterfaceLanguage(ctx);
                    NewsItem.Message message = news.getMessageByLocale(locale);
                    NotificationUtils.showNotification(serviceContext, AdsManager.AdsType.NEWS,
                            message.getTitle(), message.getBody(),
                            message.getTitle(), data
                    );
                }
            }
        }
    }

    private void checkGCMRegistration() {
        if (!Resources.getters.isAllowNetwork()) {
            ShddLog.d("shdd", "Network unavailable");
            return;
        }

        SharedPreferences prefs = AdsManager.getPreferences();
        boolean forceRegParagonCloudMsg = prefs.getBoolean(AdsManager.PREFS_KEY_FORCE_RE_REGISTER_FLAG, false);

        if (Resources.getters.isAdsAgree()) {
            String freshToken = FirebaseCm.getTokenSync();
            if (!TextUtils.isEmpty(freshToken)) {
                if (forceRegParagonCloudMsg || !paragonGcmReg.isRegisteredForToken(freshToken)) {
                    if (paragonGcmReg.register(freshToken)) {
                        prefs.edit().remove(AdsManager.PREFS_KEY_FORCE_RE_REGISTER_FLAG).apply();
                    }
                }
            } else {
                paragonGcmReg.unRegister();
            }
        } else {
            paragonGcmReg.unRegister();
        }
    }
}
