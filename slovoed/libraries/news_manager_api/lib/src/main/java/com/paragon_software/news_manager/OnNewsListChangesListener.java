package com.paragon_software.news_manager;

public interface OnNewsListChangesListener extends NewsManagerAPI.Notifier
{
	void onNewsListChanged();
}
