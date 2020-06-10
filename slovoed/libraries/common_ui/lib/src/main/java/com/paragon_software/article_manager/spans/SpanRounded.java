package com.paragon_software.article_manager.spans;

import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.NinePatchDrawable;
import androidx.annotation.NonNull;

/**
 * Copy from https://svn-shdd.paragon-software.com/svn/Projects/SlovoEd for Android/Branches/5.5/app/src/main/java/com/paragon/dictionary/ShareWordsFragment.java
 */
public class SpanRounded extends Span
{
  private NinePatchDrawable ninePatchDrawable;

  public SpanRounded( float padding, int fontColor, Resources resources, NinePatchDrawable ninePatchDrawable )
  {
    super(padding, fontColor, resources);
    this.ninePatchDrawable = ninePatchDrawable;
  }

  @Override
  public void draw( @NonNull Canvas canvas, CharSequence text, int start, int end, float x, int top, int y, int bottom,
                    @NonNull Paint paint )
  {
    float w = paint.measureText(text.subSequence(start, end).toString());
    Rect bounds = new Rect();
    bounds.set((int) ( DIP + x ), (int) (top - DIP * 2), (int) ( x + w + padding * 2 ), (int) ( bottom - DIP ));
    ninePatchDrawable.setBounds(bounds);
    ninePatchDrawable.draw(canvas);
    paint.setColor(fontColor);
    canvas.drawText(text, start, end, x + padding, y + DIP, paint);
  }
}
