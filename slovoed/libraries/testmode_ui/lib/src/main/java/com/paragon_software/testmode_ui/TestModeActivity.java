package com.paragon_software.testmode_ui;

import android.os.Bundle;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

/**
 * See {@link TestModeFragment}
 */
public class TestModeActivity extends AppCompatActivity
{
  private static final String STR_TITLE = "Test Mode";

  @Override
  protected void onCreate( @Nullable Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    setTitle(STR_TITLE);
    setContentView(R.layout.testmode_activity_layout);
  }
}
