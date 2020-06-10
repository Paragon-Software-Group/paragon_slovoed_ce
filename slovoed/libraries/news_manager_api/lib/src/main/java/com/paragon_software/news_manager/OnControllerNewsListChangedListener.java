package com.paragon_software.news_manager;


import java.util.List;

public interface OnControllerNewsListChangedListener extends NewsControllerAPI.Notifier {
	void onControllerNewsListChanged();
}
