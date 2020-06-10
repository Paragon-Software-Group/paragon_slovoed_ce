package com.paragon_software.analytics_manager.events;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.analytics_manager.Event;

abstract class TrialEventAbstract extends Event
{
  private static final String PDAHPC_ID    = "PDAHPC_ID";
  private static final String TRIAL_TYPE   = "TRIAL_TYPE";
  private static final String FEATURE_NAME = "FEATURE_NAME";

  TrialEventAbstract( @NonNull String name, @NonNull String pdahpcId, @Nullable String trialType,
                      @Nullable String featureName )
  {
    super(name);
    bundle.putString(PDAHPC_ID, pdahpcId);
    if ( null != trialType )
    {
      bundle.putString(TRIAL_TYPE, trialType);
    }
    if ( null != featureName )
    {
      bundle.putString(FEATURE_NAME, featureName);
    }
  }
}
