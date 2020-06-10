package com.paragon_software.odapi_ui;

import android.content.Context;
import android.os.Message;
import androidx.annotation.NonNull;

import com.paragon_software.analytics_manager.AnalyticsManagerAPI;
import com.paragon_software.analytics_manager.events.OdapiLaunchEvent;

public abstract class QueryWorkerAbstract extends Worker
{
  @NonNull
  final Query mQuery;

  @NonNull
  final String mValue;

  QueryWorkerAbstract( @NonNull Query query, @NonNull String value )
  {
    mQuery = query;
    mValue = value;
  }

  @Override
  public void trackAnalyticsEvent( @NonNull Message message )
  {
    Context context = getContext();
    if ( null != context )
    {
      String callerPackageName = context.getPackageManager().getNameForUid(message.sendingUid);
      String type = null != mQuery.type? mQuery.type.getName(): "";
      String specific = null != mQuery.specific? mQuery.specific.getName(): "";
      String mode = null != mQuery.mode? mQuery.mode.getName(): "";
      AnalyticsManagerAPI.get().logEvent(new OdapiLaunchEvent(type, specific, mode, callerPackageName));
    }
  }
}
