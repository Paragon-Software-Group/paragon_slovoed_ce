package com.paragon_software.utils_slovoed_ui;

import android.content.Context;
import androidx.annotation.NonNull;

public final class StringsUtils
{
  public static String createTrialLengthString( @NonNull Context context, int lengthInMinutes )
  {
    int days = Math.round(lengthInMinutes / (24f * 60));
    return context.getString(R.string.utils_slovoed_ui_trial_day_length, days);
  }
}
