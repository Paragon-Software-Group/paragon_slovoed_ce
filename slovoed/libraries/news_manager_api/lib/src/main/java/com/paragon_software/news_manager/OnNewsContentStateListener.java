package com.paragon_software.news_manager;

public interface OnNewsContentStateListener extends NewsControllerAPI.Notifier {
	void onNewsContentStateListener(boolean visibility);
}
