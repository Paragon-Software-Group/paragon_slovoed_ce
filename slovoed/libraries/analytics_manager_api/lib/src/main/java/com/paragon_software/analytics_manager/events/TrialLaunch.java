package com.paragon_software.analytics_manager.events;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class TrialLaunch extends TrialEventAbstract
{
  private static final String TRIAL_LAUNCH = "TRIAL_LAUNCH";

  public TrialLaunch( @NonNull String pdahpcId, @Nullable String trialType, @Nullable String featureName )
  {
    super(TRIAL_LAUNCH, pdahpcId, trialType, featureName);
  }
}
