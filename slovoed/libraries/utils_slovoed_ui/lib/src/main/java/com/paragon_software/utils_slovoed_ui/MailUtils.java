package com.paragon_software.utils_slovoed_ui;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.StatFs;
import android.util.DisplayMetrics;

import java.text.DecimalFormat;

import com.paragon_software.utils_slovoed.pdahpc.Catalog;
import com.paragon_software.utils_slovoed.pdahpc.PDAHPCDataParser;

public final class MailUtils
{

  private MailUtils() { }

  private static void sendMail( Context context, String to, String subject, String body )
  {
    context.startActivity(prepareMailIntent(to, subject, body));
  }

  public static void reportProblem( Context context, int engineVersion, int engineBuild )
  {
    String to = prepareMailSendTo(context);
    String subject = prepareMailSubject(context);
    String body = prepareMailTechnicalInfo(context, engineVersion, engineBuild);
    sendMail(context, to, subject, body);
  }

  private static String prepareMailSendTo( Context context )
  {
    String to = "support@slovoed.com";
    Catalog catalog = PDAHPCDataParser.parseCatalog(context);
    if ( catalog.getAbout() != null && catalog.getAbout().getSupportEmailMap() != null
        && catalog.getAbout().getSupportEmailMap().get("en") != null)
    {
      to = catalog.getAbout().getSupportEmailMap().get("en");
    }
    return to;
  }

  private static String prepareMailSubject( Context context )
  {
    String subject = context.getApplicationInfo().loadLabel(context.getPackageManager()).toString();
    Catalog catalog = PDAHPCDataParser.parseCatalog(context);
    if ( catalog.getAbout() != null && catalog.getAbout().getNameMap() != null
        && catalog.getAbout().getNameMap().get("en") != null)
    {
      subject = catalog.getAbout().getNameMap().get("en");
    }
    subject = subject + ", " + getAppVersionName(context);
    return subject;
  }

  private static String prepareMailTechnicalInfo( Context context, int engineVersion, int engineBuild )
  {
    StringBuilder builder = new StringBuilder();
    try
    {
      builder.append("\n\n");
      builder.append(context.getString(R.string.utils_slovoed_ui_support_mail_text_device_information_title)).append("\n");
      builder = insertRowInfo(context, builder, R.string.utils_slovoed_ui_support_mail_text_android_version, Build.VERSION.RELEASE);
      builder = insertRowInfo(context, builder, R.string.utils_slovoed_ui_support_mail_text_device_model,
                              Build.MANUFACTURER + " " + Build.MODEL);
      builder = insertRowInfo(context, builder, R.string.utils_slovoed_ui_support_mail_text_dictionary_version,
                              context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionName);
      builder = insertRowInfo(context, builder, R.string.utils_slovoed_ui_support_mail_text_package_name, context.getPackageName());
      DecimalFormat format = new DecimalFormat("#.##");
      if (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState()))
      {
        StatFs stat = new StatFs(Environment.getExternalStorageDirectory().getPath());
        builder = insertRowInfo(context, builder, R.string.utils_slovoed_ui_support_mail_text_external_space,
                                format.format(getAvailableSpace(stat)) + " Gb");
      }
      else
      {
        builder = insertRowInfo(context, builder, R.string.utils_slovoed_ui_support_mail_text_external_space,
                                context.getString(R.string.utils_slovoed_ui_support_mail_text_card_not_mounted));
      }
      builder = insertRowInfo(context, builder, R.string.utils_slovoed_ui_support_mail_text_internal_space, format.format(
          getAvailableSpace(new StatFs(Environment.getDataDirectory().getPath()))) + " Gb");
      DisplayMetrics metrics = context.getResources().getDisplayMetrics();
      String screen  = metrics.heightPixels + "x" + metrics.widthPixels;
      builder = insertRowInfo(context, builder, R.string.utils_slovoed_ui_support_mail_text_screen_resolution, screen);
      builder = insertRowInfo(context, builder, R.string.utils_slovoed_ui_support_mail_text_search_engine,
                              String.format(context.getString(R.string.utils_slovoed_ui_support_mail_text_search_engine_version),
                                            engineVersion, engineBuild));
    }
    catch (PackageManager.NameNotFoundException exception)
    {
      exception.printStackTrace();
    }
    return builder.toString();
  }

  private static StringBuilder insertRowInfo(Context ctx, StringBuilder builder, int resource, String data)
  {
    return builder.append(ctx.getString(resource)).append(": ").append(data).append(";\n");
  }

  private static double getAvailableSpace( StatFs stat )
  {
    double sdAvailSize = (double) stat.getAvailableBlocks()
        * (double) stat.getBlockSize();
    //One binary gigabyte equals 1,073,741,824 bytes.
    double gigaAvailable = sdAvailSize / 1073741824;
    return gigaAvailable;
  }

  private static Intent prepareMailIntent( String to, String subject, String body )
  {
    return new Intent(Intent.ACTION_SENDTO, Uri
        .parse("mailto:" + to + "?subject=" + Uri.encode(subject) + "&body=" + Uri.encode(body)));
  }

  private static String getAppVersionName( Context context )
  {
    try
    {
      return context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionName;
    }
    catch (PackageManager.NameNotFoundException e)
    {
      throw new RuntimeException("Can't receive own app version name", e);
    }
  }
}
