package com.paragon_software.settings_manager;

import android.os.Bundle;
import androidx.annotation.Nullable;

import androidx.fragment.app.Fragment;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

import com.paragon_software.common_ui.R;


public abstract class SettingsBaseActivity extends AppCompatActivity
{

  @Override
  public void onCreate( @Nullable Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_base_settings);
    setCustomTitle();

    if (savedInstanceState == null) {
      getSupportFragmentManager()
              .beginTransaction()
              .add(R.id.settings_section_fragment, getFragment()).commit();
    }

    ActionBar actionBar = getSupportActionBar();
    if ( actionBar != null )
    {
      actionBar.setDisplayHomeAsUpEnabled(true);
    }
  }

  protected abstract void setCustomTitle();

  protected abstract Fragment getFragment();
  @Override
  public boolean onSupportNavigateUp()
  {
    super.onBackPressed();
    return true;
  }
}
