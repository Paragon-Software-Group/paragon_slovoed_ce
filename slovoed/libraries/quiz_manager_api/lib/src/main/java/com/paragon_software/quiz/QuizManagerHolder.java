package com.paragon_software.quiz;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class QuizManagerHolder {

	@Nullable
	private static QuizManagerAPI mManager = null;

	@NonNull
	static QuizManagerAPI getManager() throws IllegalStateException {
		if (null == mManager) {
			throw new IllegalStateException("QuizManagerHolder uninitialized");
		}
		return mManager;
	}

	public static void setManager(@Nullable QuizManagerAPI manager) {
		mManager = manager;
	}
}
