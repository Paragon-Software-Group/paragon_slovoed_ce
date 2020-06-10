package com.paragon_software.theme_manager;

import android.content.Context;
import android.content.res.Configuration;
import android.os.Build;
import android.webkit.WebView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatDelegate;

import com.paragon_software.settings_manager.ApplicationSettings;
import com.paragon_software.settings_manager.OnApplicationSettingsSaveListener;
import com.paragon_software.settings_manager.SettingsManagerAPI;

import io.reactivex.Observable;
import io.reactivex.subjects.BehaviorSubject;

import static android.webkit.WebSettings.FORCE_DARK_OFF;
import static android.webkit.WebSettings.FORCE_DARK_ON;

public class ApplicationThemeManager implements ApplicationThemeManagerAPI, OnApplicationSettingsSaveListener
{

    private final Context mContext;
    private final BehaviorSubject<ApplicationTheme> mApplicationThemeSubject = BehaviorSubject.create();

    private boolean mIsSystemThemeEnable;

    public ApplicationThemeManager( @NonNull Context context, @Nullable SettingsManagerAPI settingsManager )
    {
        this.mContext = context;

        if(settingsManager != null)
        {
            onApplicationSettingsSaved(settingsManager.loadApplicationSettings());
            settingsManager.registerNotifier(this);
        }

        //Now we support dark mode only for Android 10 and higher
        if( Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q )
        {
            AppCompatDelegate.setDefaultNightMode(mIsSystemThemeEnable
                    ? getSystemNightMode()
                    : getLocalNightMode());
        }
    }

    @Override
    public Observable<ApplicationTheme> getApplicationTheme()
    {
        if( Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q )
        {
            return mApplicationThemeSubject;
        }
        else
        {
            return Observable.empty();
        }
    }

    @Override
    public void applyTheme(WebView webView)
    {
        if( Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q )
        {
            if (webView != null) {
                int webViewDarkMode = -1;
                switch ( mApplicationThemeSubject.getValue() )
                {
                    case Dark:
                        webViewDarkMode = FORCE_DARK_ON;
                        break;

                    case Light:
                        webViewDarkMode = FORCE_DARK_OFF;
                        break;

                    case None:
                        webViewDarkMode = isSystemNightModeEnable()
                                ? FORCE_DARK_ON
                                : FORCE_DARK_OFF;
                        break;
                }
                webView.getSettings().setForceDark(webViewDarkMode);
            }
        }
    }

    @Override
    public void onApplicationSettingsSaved( @NonNull ApplicationSettings applicationSettings )
    {
        boolean isThemeEnable = applicationSettings.isSystemThemeEnable();
        ApplicationTheme applicationTheme = applicationSettings.getTheme();

        ApplicationTheme currentApplicationTheme = mApplicationThemeSubject.getValue();

        if( !applicationTheme.equals(currentApplicationTheme) && !isThemeEnable)
        {
            mApplicationThemeSubject.onNext(applicationTheme);
        }

        if( isThemeEnable != mIsSystemThemeEnable )
        {
            mIsSystemThemeEnable = isThemeEnable;
            if(mIsSystemThemeEnable)
            {
                mApplicationThemeSubject.onNext(ApplicationTheme.None);
            }
            else
            {
                mApplicationThemeSubject.onNext(applicationTheme);
            }
        }
    }

    private boolean isSystemNightModeEnable()
    {
        int nightMode = mContext.getApplicationContext()
                .getResources()
                .getConfiguration()
                .uiMode & Configuration.UI_MODE_NIGHT_MASK;
        return nightMode == Configuration.UI_MODE_NIGHT_YES;
    }

    private int getSystemNightMode()
    {
        if(isSystemNightModeEnable())
        {
            return AppCompatDelegate.MODE_NIGHT_YES;
        }
        else
        {
            return AppCompatDelegate.MODE_NIGHT_NO;
        }
    }

    private int getLocalNightMode()
    {
        ApplicationTheme applicationTheme = mApplicationThemeSubject.getValue();
        if(applicationTheme == null)
        {
            return AppCompatDelegate.getDefaultNightMode();
        }

        int nightMode;
        switch (applicationTheme)
        {
            case Light:
            case None:
            {
                nightMode = AppCompatDelegate.MODE_NIGHT_NO;
                break;
            }
            case Dark:
            {
                nightMode = AppCompatDelegate.MODE_NIGHT_YES;
                break;
            }
            default:
            {
                throw new IllegalArgumentException("Invalid theme");
            }
        }

        return nightMode;
    }
}
