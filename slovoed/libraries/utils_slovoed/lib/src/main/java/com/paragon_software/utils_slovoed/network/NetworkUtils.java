package com.paragon_software.utils_slovoed.network;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Set;
import java.util.WeakHashMap;

public final class NetworkUtils {
    public interface OnConnected {
        void onInternetConnected();
    }

    private static final Set<OnConnected> LISTENERS = Collections.newSetFromMap(new WeakHashMap<OnConnected, Boolean>());

    private static final NetworkRequest NETWORK_REQUEST; static {
        NetworkRequest.Builder builder = new NetworkRequest.Builder();
        builder.addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET);
        NETWORK_REQUEST = builder.build();
    }

    @Nullable
    private static Callback CALLBACK = null;

    private NetworkUtils() { }

  public static boolean isNetworkAvailable( @Nullable Context context )
  {
    boolean res = false;
    if ( context != null )
    {
      ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
      if ( connectivityManager != null )
      {
        NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo();
        if ( networkInfo != null )
          res = networkInfo.isConnected();
      }
    }
    return res;
  }

    public static void registerListener(@NonNull Context context, @NonNull OnConnected listener) {
        synchronized (LISTENERS) {
            LISTENERS.add(listener);
            registerIfNeeded(context);
        }
    }

    public static void unregisterListener(@NonNull Context context, @NonNull OnConnected listener) {
        synchronized (LISTENERS) {
            LISTENERS.remove(listener);
            registerIfNeeded(context);
        }
    }

    private static void registerIfNeeded(@NonNull Context context) {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if(connectivityManager != null)
            if((!LISTENERS.isEmpty()) && (CALLBACK == null)) {
                CALLBACK = new Callback(context.getApplicationContext());
                connectivityManager.registerNetworkCallback(NETWORK_REQUEST, CALLBACK);
            }
            else if(LISTENERS.isEmpty() && (CALLBACK != null)) {
                connectivityManager.unregisterNetworkCallback(CALLBACK);
                CALLBACK = null;
            }
    }

    private static class Callback extends ConnectivityManager.NetworkCallback {
        @NonNull
        private final Context appContext;

        Callback(@NonNull Context _appContext) {
            appContext = _appContext;
        }

        @Override
        public void onAvailable(Network network) {
            ConnectivityManager connectivityManager = (ConnectivityManager) appContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            if(connectivityManager != null) {
                NetworkInfo networkInfo = connectivityManager.getNetworkInfo(network);
                if(networkInfo != null)
                    if(networkInfo.isConnected())
                        synchronized (LISTENERS) {
                            for (OnConnected listener : new ArrayList<>(LISTENERS))
                                listener.onInternetConnected();
                        }
            }
        }
    }
}
