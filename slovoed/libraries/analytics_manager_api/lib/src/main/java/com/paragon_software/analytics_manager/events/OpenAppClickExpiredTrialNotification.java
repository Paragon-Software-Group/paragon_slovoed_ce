package com.paragon_software.analytics_manager.events;

import androidx.annotation.NonNull;

public class OpenAppClickExpiredTrialNotification extends TrialEventAbstract
{
  private static final String OPEN_APP_CLICK_EXPIRED_TRIAL_NOTIFICATION = "OPEN_APP_CLICK_EXPIRED_TRIAL_NOTIF";

  public OpenAppClickExpiredTrialNotification( @NonNull String pdahpcId )
  {
    super(OPEN_APP_CLICK_EXPIRED_TRIAL_NOTIFICATION, pdahpcId, null, null);
  }
}
