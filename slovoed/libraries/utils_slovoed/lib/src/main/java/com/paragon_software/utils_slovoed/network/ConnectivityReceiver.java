package com.paragon_software.utils_slovoed.network;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.WeakHashMap;

public class ConnectivityReceiver extends BroadcastReceiver
{
    public interface OnConnected
    {
        void onInternetConnected();
    }

    private static final Set<OnConnected> mListeners = Collections.newSetFromMap(new WeakHashMap<OnConnected, Boolean>());
    private static BroadcastReceiver _instance = null;

    private boolean mIsConnected;

    public ConnectivityReceiver(Context context)
    {
        mIsConnected = NetworkUtils.isNetworkAvailable(context);
    }

    @Override
    public void onReceive(Context context, Intent intent)
    {
        boolean isConnected = NetworkUtils.isNetworkAvailable(context);
        if (ConnectivityManager.CONNECTIVITY_ACTION.equals(intent.getAction()))
        {
            if(mIsConnected != isConnected && isConnected)
            {
                mIsConnected = true;
                synchronized (mListeners) {
                    List<OnConnected> list = new ArrayList<>(mListeners);
                    for (OnConnected listener : list)
                    {
                        listener.onInternetConnected();
                    }
                }
            }
            else
            {
                mIsConnected = isConnected;
            }
        }
    }

    public static void addOnConnectedListener(Context context, OnConnected listener) {
        synchronized (mListeners) {
            mListeners.add(listener);
            registerIfNeeded(context);
        }
    }

    public static void removeConnectedListener(Context ctx, OnConnected c) {
        synchronized (mListeners) {
            mListeners.remove(c);
            registerIfNeeded(ctx);
        }
    }
    private static void registerIfNeeded(Context context) {
        context = context.getApplicationContext();
        if ((!mListeners.isEmpty()) && (_instance == null)) {
            _instance = new ConnectivityReceiver(context);
            context.registerReceiver(_instance, new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));
        } else if (mListeners.isEmpty() && (_instance != null)) {
            context.unregisterReceiver(_instance);
            _instance = null;
        }
    }
}

