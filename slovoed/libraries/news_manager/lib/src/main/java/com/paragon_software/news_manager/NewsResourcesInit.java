package com.paragon_software.news_manager;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.Bundle;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import com.paragon_software.settings_manager.SettingsManagerHolder;

import java.lang.ref.WeakReference;

import shdd.android.components.news.AdsManager;
import shdd.android.components.news.NotificationSpecs;
import shdd.android.components.news.Resources;
import shdd.android.components.news.Resources.Getters;
import shdd.android.components.news.Resources.Mode;
import shdd.android.components.news.Resources.Strings;

public final class NewsResourcesInit implements Getters {

	private final NewsNotificationOptions mNotificationOptions;
	private final String catalogOrProductId;
	private final NewsManagerAPI.Mode mMode;
	@Nullable
	private final String pkey;
	private WeakReference<Context> mContext;
	private boolean mIsTestMode;

	NewsResourcesInit(Context mContext, NewsNotificationOptions notificationOptions, String catalogOrProductId,
					  NewsManagerAPI.Mode mode, @Nullable String pkey, boolean isTestMode ) {
		this.mContext = new WeakReference<>(mContext);
		this.mNotificationOptions = notificationOptions;
		this.catalogOrProductId = catalogOrProductId;
		this.mMode = mode;
		this.pkey = pkey;
		mIsTestMode = isTestMode;
	}

	@Override
	public String getString(Strings s) {
		switch (s) {
			case HARDCODEDCONSTANTS_APPSTORE_BUILD_ADSMODIFICATIONNAME:
				return null;
//			case ADS_NOTIFICATION_BANNER_TITLE:
//				stringConstant = "Discount available";
//				break;
			case LOCALE:
				return getAppContext().getResources().getConfiguration().locale.getLanguage();
			case CATALOG_OR_PRODUCT_ID:
				return catalogOrProductId;
			default:
				return null;
		}
	}

	@Override
	public boolean isInTestMode() {
		return mIsTestMode;
	}


	void setIsTestMode(boolean isTestMode) {
		this.mIsTestMode = isTestMode;
	}


	@Override
	public boolean isAdsAgree() {
		if (SettingsManagerHolder.getManager() != null) {
			return SettingsManagerHolder.getManager().loadApplicationSettings().isNewsEnabled();
		}
		return false;
	}

	@Override
	public String getDeviceId() {
		// https://bugs.penreader.com/projects/android/wiki/%D0%9D%D0%BE%D0%B2%D0%BE%D1%81%D1%82%D0%B8_%D0%B2_%D0%BA%D0%BE%D0%BD%D1%82%D0%B5%D0%B9%D0%BD%D0%B5%D1%80%D0%B5
		return "0"; //не передаем! Но параметр обязательный, так что шлем ноль просто
	}

	@Override
	public Mode getNewsMode() {
		if (NewsManagerAPI.Mode.STANDALONE_MODE.equals(mMode))
			return Resources.Mode.STANDALONE_MODE;
		else return Resources.Mode.CONTAINER_MODE;
	}

	@Override
	public String getPrcs() {
		return null;
		//new NewsTargeting().getPrcs();
	}

	@Override
	public String getPkey()
	{
		// Read more about pkey here: https://prm-wiki.paragon-software.com/pages/viewpage.action?pageId=60268891
		return pkey;
	}

	@Override
	public Context getAppContext() {
		if (null == mContext.get()) {
			throw new IllegalStateException("Cant get application mContext");
		}
		return mContext.get();
	}

	@Override
	public NotificationSpecs getNotificationSpecs() {
		return new NotificationSpecs() {
			@RequiresApi(api = Build.VERSION_CODES.O)
			@Override
			public NotificationChannel getChannel(AdsManager.AdsType adsType) {
				return new NotificationChannel(getChannelId(adsType), "News", NotificationManager.IMPORTANCE_DEFAULT);
			}

			@Override
			public String getChannelId(AdsManager.AdsType adsType) {
				return mNotificationOptions.getChanelNews();
			}

			@Override
			public int getNotificationIcon() {
				return mNotificationOptions.getNotificationIcon();
			}

			@Override
			public int getLargeNotificationIcon() {
				return mNotificationOptions.getLargeNotificationIcon();
			}

			@Override
			public PendingIntent createPendingIntent(AdsManager.AdsType adsType, Bundle extras) {
				return PendingIntent.getActivity(mContext.get(),
						0,
						createNewsIntent(adsType, extras),
						PendingIntent.FLAG_UPDATE_CURRENT);
			}
		};
	}

	@Override
	public boolean isAllowNetwork() {

		boolean res = false;
		ConnectivityManager
				connectivityManager = (ConnectivityManager) getAppContext().getSystemService(Context.CONNECTIVITY_SERVICE);
		if (connectivityManager != null) {
			NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo();
			if (networkInfo != null)
				res = networkInfo.isConnected();
		}
		return res;
	}

	private Intent createNewsIntent(AdsManager.AdsType adsType, Bundle extras) {
		return new Intent(getAppContext(), mNotificationOptions.getActivity())
				.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_CLEAR_TOP)
				.putExtra(AdsManager.INTENT_KEY_ADS_FROM_NOTIFICATION, true)
				.putExtra(AdsManager.INTENT_KEY_ADS_TYPE, adsType)
				.putExtras(extras);
	}
}
