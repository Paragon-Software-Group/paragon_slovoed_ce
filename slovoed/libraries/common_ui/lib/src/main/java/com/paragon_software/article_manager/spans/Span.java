package com.paragon_software.article_manager.spans;

import android.content.res.Resources;
import android.graphics.Paint;
import androidx.annotation.NonNull;
import android.text.style.ReplacementSpan;
import android.util.TypedValue;

/**
 * Copy from https://svn-shdd.paragon-software.com/svn/Projects/SlovoEd for Android/Branches/5.5/app/src/main/java/com/paragon/dictionary/ShareWordsFragment.java
 */
public abstract class Span extends ReplacementSpan
{
  final float DIP;
  final float padding;
  final int   fontColor;

  Span( float padding, int fontColor, Resources resources )
  {
    DIP = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 1, resources.getDisplayMetrics());
    this.padding = padding;
    this.fontColor = fontColor;
  }

  @Override
  public int getSize( @NonNull Paint paint, CharSequence text, int start, int end, Paint.FontMetricsInt fm )
  {
    return (int) ( DIP + padding * 2 + paint.measureText(text.subSequence(start, end).toString()) );
  }
}
