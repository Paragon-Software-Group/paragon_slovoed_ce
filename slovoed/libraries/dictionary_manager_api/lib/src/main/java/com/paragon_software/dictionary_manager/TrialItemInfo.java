package com.paragon_software.dictionary_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class TrialItemInfo
{
  private static final long SLOVOED_TRIAL_CONTINUE_INTERVAL = 30000L;

  @NonNull
  private final FeatureName mEntity;
  @Nullable
  private final Long   mStartTime;
  @Nullable
  private final Long   mEndTime; //in ms since epoch

  public TrialItemInfo(@NonNull FeatureName entity, @Nullable Long startTime, @Nullable Long endTime )
  {
    mEntity = entity;
    mStartTime = startTime;
    mEndTime = endTime;
  }

  @NonNull
  public FeatureName getEntity()
  {
    return mEntity;
  }

  @Nullable
  public Long getStartTime()
  {
    return mStartTime;
  }

  @Nullable
  public Long getEndTime()
  {
    return mEndTime;
  }

  public boolean isExpired()
  {
    return (mEndTime != null) && (mEndTime < System.currentTimeMillis());
  }

  public boolean isContinue()
  {
    return (mStartTime != null) && (System.currentTimeMillis() - mStartTime > SLOVOED_TRIAL_CONTINUE_INTERVAL);
  }
}
