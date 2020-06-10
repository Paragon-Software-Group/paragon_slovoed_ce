package com.paragon_software.dictionary_manager_ui.search;

import android.os.Handler;
import androidx.annotation.NonNull;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;

public class TextChangeListenerDelayed implements TextWatcher
{
  private static final long DEFAULT_DELAY = 200;
  private final Handler  handler;
  private final Listener listener;
  private       long     delayMillis;
  private       Runnable runnable;

  public TextChangeListenerDelayed( Listener listener )
  {
    this(DEFAULT_DELAY, listener);
  }

  public TextChangeListenerDelayed( long delayMillis, Listener listener )
  {
    this.delayMillis = delayMillis;
    this.listener = listener;
    this.handler = new Handler();
  }

  @Override
  public void beforeTextChanged( CharSequence charSequence, int i, int i1, int i2 )
  {
  }

  @Override
  public void onTextChanged( CharSequence charSequence, int i, int i1, int i2 )
  {
  }

  @Override
  public void afterTextChanged( final Editable editable )
  {
    if ( null != runnable )
    {
      handler.removeCallbacks(runnable);
    }

    runnable = new Runnable()
    {
      @Override
      public void run()
      {
        listener.afterTextChanged(editable);
        TextChangeListenerDelayed.this.runnable = null;
      }
    };
    if ( TextUtils.isEmpty(editable.toString()) )
    {
      runnable.run();
    }
    else
    {
      handler.postDelayed(runnable, delayMillis);
    }
  }

  public long getDelayMillis()
  {
    return delayMillis;
  }

  public void setDelayMillis( long delayMillis )
  {
    this.delayMillis = delayMillis;
  }

  public interface Listener
  {
    void afterTextChanged( @NonNull Editable editable );
  }
}
