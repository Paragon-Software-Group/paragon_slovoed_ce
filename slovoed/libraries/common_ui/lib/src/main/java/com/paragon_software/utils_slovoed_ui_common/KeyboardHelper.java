package com.paragon_software.utils_slovoed_ui_common;

import android.content.Context;
import android.os.Handler;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class KeyboardHelper
{
	private static boolean appStartup = true;
	private static Runnable mShowKeyboardTask = null;

	public static void showSoftwareKeyboard(final @Nullable View view, @NonNull Handler handler )
	{
		/*
		1000 ms delay is needed to show keyboard at app startup,
		300 ms in other cases (hide/show app, orientation change)
		*/
		int delay = appStartup ? 1000 : 300;

		tryClearShowKeyboardTask(handler);

		mShowKeyboardTask = () -> {
			if ( view != null && view.requestFocus() )
			{
				InputMethodManager imm = (InputMethodManager) view.getContext()
						.getSystemService(Context.INPUT_METHOD_SERVICE);

				// InputMethodManager.SHOW_FORCED to show keyboard even in landscape orientation
				if (imm != null)
					imm.showSoftInput(view, InputMethodManager.SHOW_FORCED);
			}
		};
		handler.postDelayed(mShowKeyboardTask, delay);

		appStartup = false;
	}

	public static void hideSoftwareKeyboard( @Nullable View view, @NonNull Handler handler )
	{
		tryClearShowKeyboardTask(handler);

		if (view != null)
		{
			InputMethodManager imm = (InputMethodManager) view.getContext()
					.getSystemService(Context.INPUT_METHOD_SERVICE);

			if (imm != null)
				imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
		}
		appStartup = false;
	}

	private static void tryClearShowKeyboardTask( @NonNull Handler handler )
	{
		if (mShowKeyboardTask != null)
			handler.removeCallbacks(mShowKeyboardTask);
		mShowKeyboardTask = null;
	}
}