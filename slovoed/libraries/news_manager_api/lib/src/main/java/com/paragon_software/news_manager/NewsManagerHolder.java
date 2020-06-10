package com.paragon_software.news_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class NewsManagerHolder {
	@Nullable
	private static NewsManagerAPI mManager = null;

	@Nullable
	public static NewsManagerAPI getManager() {
		return mManager;
	}

	public static void setManager(@NonNull NewsManagerAPI manager) {
		mManager = manager;
	}
}
