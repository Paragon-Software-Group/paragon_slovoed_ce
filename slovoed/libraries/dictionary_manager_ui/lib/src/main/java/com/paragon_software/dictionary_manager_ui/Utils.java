package com.paragon_software.dictionary_manager_ui;

import androidx.annotation.IdRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.SparseArray;
import android.view.View;
import android.widget.ImageView;

import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.dictionary_manager.exceptions.NotALocalResourceException;

import java.util.Locale;

public final class Utils
{
  public static void setDictionaryIcon( @Nullable ImageView imageView, @Nullable IDictionaryIcon icon )
  {
    if ( imageView != null )
    {
      if ( icon != null )
      {
        try
        {
          imageView.setImageResource(icon.getResourceId());
        }
        catch ( NotALocalResourceException ignore )
        {
          imageView.setImageBitmap(icon.getBitmap());
        }
      }
      else
      {
        imageView.setImageDrawable(null);
      }
    }
  }

  /**
   * @param price Price in micro-units, where 1,000,000 micro-units equal one unit of the currency. Example: 7990000 = 7.99
   */
  public static String convertPriceToString( long price )
  {
    return String.format(Locale.ENGLISH,"%.2f", price/1000000.d);
  }

  private static final long KiB = 1024;
  private static final long MiB = 1024 * KiB;
  private static final long GiB = 1024 * MiB;

  private static String bytesSizeStrKB_MB_GB( long size ) {
    if (size == Utils.KiB) {
      return " KB";
    } else if (size == Utils.MiB) {
      return " MB";
    } else if (size == Utils.GiB) {
      return " GB";
    }
    throw new IllegalArgumentException("Undefined string for size [" + size + "]");
  }

  public static String formatSize(long size) {
    final String mask = "%.1f";
    if (size < 0) return "0";
    if (size < Utils.KiB) return size + " B";
    if (size < Utils.MiB) return size / Utils.KiB + bytesSizeStrKB_MB_GB(Utils.KiB);
    if (size < Utils.GiB)
      return String.format(Locale.ENGLISH, mask, (double) size / Utils.MiB) + bytesSizeStrKB_MB_GB(Utils.MiB);
    return String.format(Locale.ENGLISH, mask, (double) size / Utils.GiB) + bytesSizeStrKB_MB_GB(Utils.GiB);
  }

  private static final long SECOND_IN_MILLIS = 1000;
  private static final long MINUTE_IN_MILLIS = 60 * SECOND_IN_MILLIS;
  private static final long HOUR_IN_MILLIS   = 60 * MINUTE_IN_MILLIS;

  public static String formatDuration( long millis )
  {
    if ( millis >= HOUR_IN_MILLIS )
    {
      final int hours = (int) ( ( millis + 1800000 ) / MINUTE_IN_MILLIS );
      return hours + "h";
    }
    else if ( millis >= MINUTE_IN_MILLIS )
    {
      final int minutes = (int) ( ( millis + 30000 ) / MINUTE_IN_MILLIS );
      return minutes + "m";
    }
    else
    {
      final int seconds = (int) ( ( millis + 500 ) / SECOND_IN_MILLIS );
      return seconds + "s";
    }
  }

  public static void setVisibility(@NonNull SparseArray< View > viewSparseArray, @IdRes int id, int visibility )
  {
      View view = viewSparseArray.get(id);
      if (null != view && id == view.getId())
      {
        view.setVisibility(visibility);
      }
  }
}
