package com.paragon_software.odapi_ui;

import android.app.Activity;
import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Messenger;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.SyncEngineAPI;
import com.paragon_software.splash_screen_manager.ISplashScreenManager;

public class OpenDictionaryAPIService extends Service {
    public static void registerDictionaryManager(@NonNull DictionaryManagerAPI dictionaryManager) {
        Worker.registerDictionaryManager(dictionaryManager);
    }

    public static void registerEngine(@NonNull SyncEngineAPI engine){
        Worker.registerEngine(engine);
    }

    public static void registerArticleManager(@NonNull ArticleManagerAPI articleManager) {
        Worker.registerArticleManager(articleManager);
    }

    public static void registerArticleActivity(@NonNull Class<? extends Activity> activityClass) {
        Worker.registerArticleActivity(activityClass);
    }

    public static void registerSplashscreenManager(@NonNull ISplashScreenManager splashScreenManager) {
        Worker.registerSplashscreenManager(splashScreenManager);
    }

    @NonNull
    private final Handler mHandler = new IncomingHandler();

    @NonNull
    private final Messenger mMessenger = new Messenger(mHandler);

    @Override
    public void onCreate() {
        super.onCreate();
        Worker.setContext(this);
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return mMessenger.getBinder();
    }
}
