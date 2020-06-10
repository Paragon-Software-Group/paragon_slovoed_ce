package com.paragon_software.user_core_manager;

import android.app.Activity;
import android.content.Context;

import com.paragon_software.settings_manager.SettingsManagerAPI;

import java.util.List;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import io.reactivex.Observable;

public abstract class UserCoreManagerAPI
{
  public interface Factory
  {
    @NonNull
    UserCoreManagerAPI create();

    Factory registerUi( @Nullable Class userCoreActivity );

    Factory registerSettingsManager( @Nullable SettingsManagerAPI settingManager );

    Factory registerApplicationContext(@Nullable Context context);

    Factory registerLoginDialog(@Nullable DialogFragment loginDialog);

    Factory registerResetCodeDialog(@Nullable DialogFragment resetDialog);
  }

  public static int LOGIN_RESULT_OK = 0;
  public static int LOGIN_RESULT_NO_INTERNET = 1;
  public static int LOGIN_RESULT_LICENSES_EXPIRED = 2;
  public static int LOGIN_RESULT_NO_LICENSES = 3;
  public static int LOGIN_RESULT_GENERIC_ERROR = 4;
  public static String EXTRA_CONTROLLER_ID = "EXTRA_CONTROLLER_ID";

  public abstract void login(@NonNull Activity activity, int requestCode);

  public abstract void login(@NonNull Fragment fragment, int requestCode);

  public abstract void login(@NonNull FragmentManager fragmentManager);

  public abstract void logout(@NonNull Context context );
  
  abstract void saveLoginPass(String login, String pass);
  
  public abstract void resetUserPassword(@NonNull FragmentManager fragmentManager);
  
  @NonNull
  public abstract Observable< List< ProductLicense > > getProductLicenses();

  @NonNull
  abstract UserCoreControllerAPI getController( @NonNull String controllerType );

  abstract void updateLicences(@NonNull List<ProductLicense> licenses, @NonNull String jwt);
  
  protected abstract void removeLoginPass();
  
  static class LoginException extends Exception {
    private final int resultCode;

    LoginException(int _resultCode) {
      resultCode = _resultCode;
    }

    int getResultCode() {
      return resultCode;
    }

    @NonNull
    @Override
    public String toString() {
      return "Login exception, result code: " + resultCode;
    }
  }
}
