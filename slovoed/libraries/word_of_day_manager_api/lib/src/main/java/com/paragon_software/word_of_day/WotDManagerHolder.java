package com.paragon_software.word_of_day;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class WotDManagerHolder {

	@Nullable
	private static WotDManagerAPI mManager = null;

	@NonNull
	static WotDManagerAPI getManager() throws IllegalStateException {
		if (null == mManager) {
			throw new IllegalStateException("WotDManagerHolder uninitialized");
		}
		return mManager;
	}

	public static void setManager(@Nullable WotDManagerAPI manager) {
		mManager = manager;
	}
}
