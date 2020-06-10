package com.paragon_software.theme_manager;

import android.webkit.WebView;

import io.reactivex.Observable;

public interface ApplicationThemeManagerAPI
{
    Observable<ApplicationTheme> getApplicationTheme();
    void applyTheme(WebView webView);
}
