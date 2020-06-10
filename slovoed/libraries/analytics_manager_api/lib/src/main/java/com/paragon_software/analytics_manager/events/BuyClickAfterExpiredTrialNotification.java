package com.paragon_software.analytics_manager.events;

import androidx.annotation.NonNull;

public class BuyClickAfterExpiredTrialNotification extends TrialEventAbstract
{
  private static final String BUY_CLICK_TRIAL_EXPIRED_NOTIFICATION = "BUY_CLICK_AFTER_TRIAL_EXPIRED_NOTIF";

  public BuyClickAfterExpiredTrialNotification( @NonNull String pdahpcId )
  {
    super(BUY_CLICK_TRIAL_EXPIRED_NOTIFICATION, pdahpcId, null, null);
  }
}
