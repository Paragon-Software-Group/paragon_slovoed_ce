package com.paragon_software.news_manager;

import android.app.Activity;

public class NewsNotificationOptions {

	private String chanelNews;
	private int notificationIcon;
	private int largeNotificationIcon;
	private Class<? extends Activity> activity;

	public NewsNotificationOptions(String chanelNews, int notificationIcon, int largeNotificationIcon, Class<? extends Activity> activity) {
		this.chanelNews = chanelNews;
		this.notificationIcon = notificationIcon;
		this.largeNotificationIcon = largeNotificationIcon;
		this.activity = activity;
	}

	String getChanelNews() {
		return chanelNews;
	}

	int getNotificationIcon() {
		return notificationIcon;
	}

	int getLargeNotificationIcon() {
		return largeNotificationIcon;
	}

	Class<? extends Activity> getActivity() {
		return activity;
	}
}
