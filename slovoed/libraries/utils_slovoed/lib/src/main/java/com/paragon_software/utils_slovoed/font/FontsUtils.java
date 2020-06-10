package com.paragon_software.utils_slovoed.font;

import android.content.Context;
import android.graphics.Typeface;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.HashMap;
import java.util.Map;

import android.text.SpannableString;
import android.text.Spanned;
import android.widget.TextView;
import com.google.gson.reflect.TypeToken;
import com.paragon_software.utils_slovoed.json.JsonUtils;

public final class FontsUtils
{
  @NonNull
  private static Map< String, Font > availableFonts = new HashMap<>();

  public static void init( @NonNull Context context)
  {
    Map< String, Font > fonts = JsonUtils.parseRawJsonFile(context, "fonts", new TypeToken< HashMap< String, Font > >(){});
    if ( fonts != null )
    {
      availableFonts = fonts;
      for ( Font font : availableFonts.values() )
      {
        Typeface typeface = Typeface.DEFAULT;
        if ( context.getAssets() != null )
        {
          try
          {
            typeface = Typeface.createFromAsset(context.getAssets(), font.getFilePath());
          }
          catch ( RuntimeException ignore ) { }
        }
        font.setTypeface(typeface);
      }
    }
  }

  /**
   * Try to get font for specified font name.
   * @param fontName font name to looking for
   * @return corresponding font if it is available, otherwise return null
   */
  @Nullable
  public static Font getFont( @NonNull FontName fontName )
  {
    return availableFonts.get(fontName.name);
  }

  // used in HtmlBuilder
  public static Font[] getAvailableFonts()
  {
    return availableFonts.values().toArray(new Font[0]);
  }

  public static void setFontToTextViews(FontsUtils.FontName fontName, TextView... views) {
    Font font = FontsUtils.getFont(fontName);
    if (font != null) {
      for (TextView view : views) {
        view.setTypeface(font.getTypeface());
      }
    }
  }

  public static SpannableString setCustomFontForPartOfText(SpannableString phrase, String partOfText, FontsUtils.FontName fontName) {
    Font font = FontsUtils.getFont(fontName);
    int start = 0, end;
    if (font != null) {
      while ((start = phrase.toString().indexOf(partOfText, start)) != -1) {
        end = start + partOfText.length();
        phrase.setSpan(new CustomTypefaceSpan(font.getTypeface()), start++, end, Spanned.SPAN_EXCLUSIVE_INCLUSIVE);
      }
    }
    return phrase;
  }

  // Supported fonts
  public enum FontName
  {
    AUGEAN("Augean"),
    CHARIS_SIL("Charis SIL"),
    DEJA_VU_SANS("DejaVu Sans"),
    DEJA_VU_SANS_LIGHT("DejaVu Sans Light"),
    SOURCE_SANS_PRO("Source Sans Pro"),
    SOURCE_SANS_PRO_BOLD("Source Sans Pro Bold"),
    SOURCE_SANS_PRO_SEMIBOLD("Source Sans Pro SemiBold"),
    SOURCE_SANS_PRO_ITALIC("Source Sans Pro Italic"),
    SOURCE_SANS_PRO_BOLD_ITALIC("Source Sans Pro Bold Italic"),
    SOURCE_SANS_PRO_SEMIBOLD_ITALIC("Source Sans Pro SemiBold Italic");

    private final String name;

    FontName( String _name )
    {
      name = _name;
    }
  }
}
