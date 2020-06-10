package com.paragon_software.toolbar_manager;

import androidx.annotation.Nullable;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.ActionBarDrawerToggle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

public interface BaseParagonToolbarAPI {
  void onResume();
  void onPause();
  boolean onCreateOptionsMenu( Menu menu, MenuInflater inflater );
  boolean onPrepareOptionsMenu( Menu menu );
  boolean onOptionsItemSelected( MenuItem item );
  void setActionbar( @Nullable ActionBar actionbar );
  void setDrawerLayout( @Nullable DrawerLayout drawerLayout );
  void setToggle( @Nullable ActionBarDrawerToggle toggle );
}
