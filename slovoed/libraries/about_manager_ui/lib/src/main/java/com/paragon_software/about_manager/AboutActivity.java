package com.paragon_software.about_manager;

import android.os.Bundle;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

import com.paragon_software.about_manager_ui.R;


public class AboutActivity extends AppCompatActivity
{
  @Override
  protected void onCreate( Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    setCustomTheme();
    setContentView(R.layout.activity_about);
    if ( null == savedInstanceState )
    {
      String controllerId;
      Bundle extras = getIntent().getExtras();
      controllerId = null != extras ? extras.getString(AboutManagerAPI.EXTRA_CONTROLLER_ID) : null;
      if ( null != controllerId )
      {
        AboutFragment aboutFragment = new AboutFragment();
        Bundle arguments = new Bundle();
        arguments.putString(AboutManagerAPI.EXTRA_CONTROLLER_ID, controllerId);
        aboutFragment.setArguments(arguments);
        getSupportFragmentManager().beginTransaction().add(R.id.fragment_frame, aboutFragment).commit();
      }
    }
    customizationActionBar();
  }

  protected void setCustomTheme() {
    this.setTheme(R.style.SlovoedAboutActivityTheme);
  }

  protected void customizationActionBar()
  {
    ActionBar actionBar = getSupportActionBar();
    if ( actionBar != null )
    {
      actionBar.setDisplayShowTitleEnabled(false);
      actionBar.setDisplayHomeAsUpEnabled(true);
    }
  }

  @Override
  public boolean onSupportNavigateUp()
  {
    super.onBackPressed();
    return true;
  }
}
