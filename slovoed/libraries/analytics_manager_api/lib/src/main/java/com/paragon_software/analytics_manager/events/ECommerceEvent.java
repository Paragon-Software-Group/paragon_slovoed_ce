package com.paragon_software.analytics_manager.events;

import androidx.annotation.NonNull;

import com.paragon_software.analytics_manager.Event;

public class ECommerceEvent extends Event
{
  private static final String ECOMMERCE_PURCHASE = "ecommerce_purchase";
  // parameters
  private static final String TRANSACTION_ID     = "transaction_id";
  private static final String CURRENCY           = "currency";
  private static final String VALUE              = "value";
  private static final String INAPP_ID           = "inapp_id";

  public ECommerceEvent( @NonNull String inAppId, @NonNull String transactionId, @NonNull String currency,
                         double value )
  {
    super(ECOMMERCE_PURCHASE);
    bundle.putString(INAPP_ID, inAppId);
    bundle.putString(TRANSACTION_ID, transactionId);
    bundle.putString(CURRENCY, currency);
    bundle.putDouble(VALUE, value);
  }
}
