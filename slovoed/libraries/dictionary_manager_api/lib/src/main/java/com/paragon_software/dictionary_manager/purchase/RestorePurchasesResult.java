package com.paragon_software.dictionary_manager.purchase;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.errors.ErrorType;

public class RestorePurchasesResult
{
  @NonNull
  private final RestorePurchasesResultType type;
  @Nullable
  private final ErrorType errorType;

  private RestorePurchasesResult( @NonNull final Builder builder )
  {
    type = builder.type;
    errorType = builder.errorType;
  }

  @NonNull
  public RestorePurchasesResultType getType()
  {
    return type;
  }

  @Nullable
  public ErrorType getErrorType()
  {
    return errorType;
  }

  public static class Builder
  {
    @NonNull
    private final RestorePurchasesResultType type;
    @Nullable
    private ErrorType errorType;

    public Builder( @NonNull RestorePurchasesResultType _type )
    {
      type = _type;
    }

    public Builder setErrorType( @Nullable final ErrorType _errorType )
    {
      errorType = _errorType;
      return this;
    }

    @NonNull
    public RestorePurchasesResult build()
    {
      return new RestorePurchasesResult(this);
    }
  }
}
