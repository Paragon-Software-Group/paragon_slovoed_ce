package com.paragon_software.analytics_manager.events;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.analytics_manager.Event;

public class OdapiLaunchEvent extends Event
{
  private static final String ODAPI_LAUNCH        = "ODAPI_LAUNCH";
  private static final String REQUEST_SPECS       = "REQUEST_SPECS";
  private static final String CLIENT_PACKAGE_NAME = "CLIENT_PACKAGE_NAME";

  public OdapiLaunchEvent( @NonNull String type, @NonNull String specific, @NonNull String mode,
                           @Nullable String callerPackageName )
  {
    super(ODAPI_LAUNCH);
    bundle.putString(REQUEST_SPECS, type + ";" + specific + ";" + mode);
    if ( null != callerPackageName )
    {
      bundle.putString(CLIENT_PACKAGE_NAME, callerPackageName);
    }

  }

  public OdapiLaunchEvent( @NonNull String error, @NonNull String message, @Nullable String callerPackageName )
  {
    super(ODAPI_LAUNCH);
    bundle.putString(REQUEST_SPECS, error + ";" + message);
    if ( null != callerPackageName )
    {
      bundle.putString(CLIENT_PACKAGE_NAME, callerPackageName);
    }
  }
}
