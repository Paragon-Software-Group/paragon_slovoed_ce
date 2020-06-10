package com.paragon_software.dictionary_manager;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

import java.util.HashMap;

public class Utils
{
  private static HashMap<String, String[]> assetsFilesCache = new HashMap<String, String[]>(10);

  public static String[] listAssetsFiles( Context ctx, String path ) {
    if (assetsFilesCache.containsKey(path)) return assetsFilesCache.get(path);
    try {
      String[] result = ctx.getAssets().list(path);
      assetsFilesCache.put(path, result);
      return result;
    } catch (Exception e) {
      e.printStackTrace();
      return new String[]{};
    }
  }

  public static String findAssetsFile(Context ctx, String path, String filename) {
    String[] files = listAssetsFiles(ctx, path);
    for (String file : files) {
      String cmp = file.toLowerCase();
      if (cmp.equalsIgnoreCase(filename))
        return path + "/" + file;
    }
    return null;
  }

  public static boolean isNetworkAvailable(Context context) {
    boolean res = false;
    ConnectivityManager
        connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
    if(connectivityManager != null) {
      NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo();
      if (networkInfo != null)
        res = networkInfo.isConnected();
    }
    return res;
  }
}
