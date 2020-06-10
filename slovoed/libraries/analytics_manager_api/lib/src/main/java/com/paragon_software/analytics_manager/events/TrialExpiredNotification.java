package com.paragon_software.analytics_manager.events;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class TrialExpiredNotification extends TrialEventAbstract
{
  private static final String TRIAL_EXPIRED_NOTIFICATION = "TRIAL_EXPIRED_NOTIFICATION";

  public TrialExpiredNotification( @NonNull String pdahpcId, @Nullable String trialType, @Nullable String featureName )
  {
    super(TRIAL_EXPIRED_NOTIFICATION, pdahpcId, trialType, featureName);
  }
}
