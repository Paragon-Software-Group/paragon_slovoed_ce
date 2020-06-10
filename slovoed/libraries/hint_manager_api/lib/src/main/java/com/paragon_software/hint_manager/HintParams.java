package com.paragon_software.hint_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class HintParams
{

  @Nullable
  private final Runnable onFirstActionRunnable;

  @Nullable
  private final Runnable onDismissRunnable;

  private HintParams( @NonNull final Builder builder )
  {
    onFirstActionRunnable = builder.onFirstActionRunnable;
    onDismissRunnable = builder.onDismissRunnable;
  }

  @Nullable
  public Runnable getOnFirstActionRunnable()
  {
    return onFirstActionRunnable;
  }

  @Nullable
  public Runnable getOnDismissRunnable()
  {
    return onDismissRunnable;
  }



  public static class Builder
  {

    @Nullable
    private Runnable onFirstActionRunnable;

    @Nullable
    private Runnable onDismissRunnable;

    public Builder setOnFirstActionRunnable( @Nullable final Runnable _onFirstActionRunnable )
    {
      onFirstActionRunnable = _onFirstActionRunnable;
      return this;
    }

    public Builder setOnDismissRunnable( @Nullable final Runnable _onDismissRunnable )
    {
      onDismissRunnable = _onDismissRunnable;
      return this;
    }

    @NonNull
    public HintParams build()
    {
      return new HintParams(this);
    }

  }
}
