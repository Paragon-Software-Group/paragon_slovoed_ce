package com.paragon_software.word_of_day;

import android.app.Activity;
import android.graphics.Bitmap;
import androidx.annotation.DrawableRes;
import androidx.annotation.NonNull;

public class WotDNotificationOptions
{
    @NonNull
    final Class<? extends Activity> activity;

    @DrawableRes
    final int smallIconResId;

    @NonNull
    final Bitmap largeIcon;

    WotDNotificationOptions(@NonNull Class<? extends Activity> _activity,
                            @DrawableRes int _smallIconResId,
                            @NonNull Bitmap _largeIcon)
    {
        activity = _activity;
        smallIconResId = _smallIconResId;
        largeIcon = _largeIcon;
    }
}
