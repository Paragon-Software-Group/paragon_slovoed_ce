package com.paragon_software.dictionary_manager_ui.view;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.drawable.ClipDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.LayerDrawable;
import androidx.appcompat.widget.AppCompatButton;
import android.util.AttributeSet;
import android.view.Gravity;

import com.paragon_software.dictionary_manager_ui.R;

/**
 * Custom {@link AppCompatButton} which can show progress on its background view.
 * <p>
 * NOTE: To show progress correctly it should have custom background in not 9-patch format
 * (by default {@link AppCompatButton} has 9-patch background so it should be overridden)
 */
public class ProgressBarAppCompatButton extends AppCompatButton
{

  /**
   * Initial button background.
   */
  private final Drawable baseBackground = getBackground();

  /**
   * Color for background progress line.
   */
  private int progressColor = Color.GRAY;

  public ProgressBarAppCompatButton( Context context, AttributeSet attrs )
  {
    super(context, attrs);
    applyAttributeSet(attrs, 0);
  }

  public ProgressBarAppCompatButton( Context context, AttributeSet attrs, int defStyleAttr )
  {
    super(context, attrs, defStyleAttr);
    applyAttributeSet(attrs, defStyleAttr);
  }

  private void applyAttributeSet( AttributeSet attrs, int defStyleAttr )
  {
    TypedArray typedArray =
        getContext().getTheme().obtainStyledAttributes(attrs, R.styleable.ProgressBarAppCompatButton, 0, defStyleAttr);
    try
    {
      progressColor = typedArray.getColor(R.styleable.ProgressBarAppCompatButton_progressColor, Color.GRAY);
    }
    finally
    {
      typedArray.recycle();
    }
  }

  /**
   * Update background view to show progress line according to specified progress value.
   *
   * @param newProgress the new progress, between 0 and 100
   */
  public void setProgress( int newProgress )
  {
    int progress = newProgress < 0 ? 0 : ( newProgress > 100 ? 100 : newProgress );
    ClipDrawable progressBackground =
        new ClipDrawable(baseBackground.getConstantState().newDrawable().mutate(), Gravity.LEFT,
                         ClipDrawable.HORIZONTAL);
    progressBackground.setLevel(progress * 100);
    progressBackground.setTint(progressColor);
    LayerDrawable layerDrawable = new LayerDrawable(new Drawable[]{ baseBackground.mutate(), progressBackground });
    setBackground(layerDrawable);
  }
}
