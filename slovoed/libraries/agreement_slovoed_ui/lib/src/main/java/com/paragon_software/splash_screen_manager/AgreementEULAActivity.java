package com.paragon_software.splash_screen_manager;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import android.view.View;

import com.paragon_software.agreement_screen_manager.AgreementBaseActivity;
import com.paragon_software.agreement_screen_manager.AgreementBaseEULAActivity;
import com.paragon_software.agreement_slovoed_ui.R;
import com.paragon_software.information_manager.InformationManagerFactoryAPI;

public class AgreementEULAActivity extends AgreementBaseEULAActivity
{
  public static class SplashScreenFactory implements ISplashScreen.Factory
  {
    @Nullable
    @Override
    public ISplashScreen create( @Nullable Context context )
    {
      return context == null ? null :
             new SplashScreenActivity(AgreementEULAActivity.getName(context), AgreementEULAActivity.class);
    }
  }

  public static class InformationScreenFactory implements InformationManagerFactoryAPI.InformationScreenFactory
  {
    @Nullable
    @Override
    public Intent createIntent( @Nullable Context context )
    {
      if ( context != null )
      {
        Intent eulaIntent = new Intent(context, AgreementEULAActivity.class);
        eulaIntent.putExtra(AgreementBaseActivity.HIDE_BUTTONS_EXTRA, true);
        eulaIntent.putExtra(AgreementBaseActivity.HIDE_ACTIONBAR_EXTRA, false);
        return eulaIntent;
      }
      return null;
    }
  }

  @Override
  protected void onCreate( Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    ActionBar actionBar = getSupportActionBar();
    if ( actionBar != null )
    {
      actionBar.setTitle(R.string.utils_slovoed_ui_eula);
    }
  }

  @NonNull
  @Override
  protected View.OnClickListener getAgreeButtonClickListener()
  {
    return new View.OnClickListener()
    {
      @Override
      public void onClick( View view )
      {
        ISplashScreenManager splashScreenManager = SplashScreenManagerHolder.getManager();
        splashScreenManager.showNext(AgreementEULAActivity.this);
        AgreementEULAActivity.this.finish();
      }
    };
  }

  /**
   * TODO: Concatenate getCanonicalName() & R.string.agreement_screen_manager_ui_last_updated_eula on next update EULA
   */
  public static String getName( Context context )
  {
    return AgreementEULAActivity.class.getCanonicalName();
  }
}
