package com.paragon_software.dictionary_manager.download_manager;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;

import java.util.Locale;

/**
 * Implementation form Slovoed 5.5
 */
public class HttpHeaders
{

  public static final String USER_AGENT_HEADER_NAME = "User-Agent";

  private HttpHeaders()
  {
  }

  public static String getUserAgentHeaderValue( Context context )
  {
    return String.format(Locale.US, "Android/%s; %s/%s; %s/%s", Build.VERSION.RELEASE, Build.MANUFACTURER, Build.MODEL,
                         context.getPackageName(), getAppVersion(context));
  }

  private static String getAppVersion( Context context )
  {
    try
    {
      return context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionName;
    }
    catch ( Throwable ignore )
    {
      return "unknown";
    }
  }
}
