package com.paragon_software.utils_slovoed.font;

import android.graphics.Paint;
import android.graphics.Typeface;
import android.text.TextPaint;
import android.text.style.MetricAffectingSpan;

public class CustomTypefaceSpan extends MetricAffectingSpan {

  private final Typeface typeface;

  public CustomTypefaceSpan(Typeface typeface) {
    this.typeface = typeface;
  }

  @Override
  public void updateDrawState(TextPaint ds) {
    applyCustomTypeFace(ds, typeface);
  }

  @Override
  public void updateMeasureState(TextPaint paint) {
    applyCustomTypeFace(paint, typeface);
  }

  private static void applyCustomTypeFace(Paint paint, Typeface tf) {
    paint.setTypeface(tf);
  }
}