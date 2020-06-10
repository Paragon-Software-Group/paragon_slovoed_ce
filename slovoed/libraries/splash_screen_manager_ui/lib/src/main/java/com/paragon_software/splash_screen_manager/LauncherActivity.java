package com.paragon_software.splash_screen_manager;

import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;


public class LauncherActivity extends AppCompatActivity
{
  @Override
  protected void onCreate( Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    ISplashScreenManager splashScreenManager = SplashScreenManagerHolder.getManager();
    splashScreenManager.reset();
    splashScreenManager.showNext(this);
    finish();
  }
}