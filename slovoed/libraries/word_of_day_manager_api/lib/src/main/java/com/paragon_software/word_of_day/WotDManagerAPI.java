package com.paragon_software.word_of_day;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import androidx.annotation.DrawableRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.hint_manager.HintManagerAPI;
import com.paragon_software.settings_manager.SettingsManagerAPI;

import com.paragon_software.screen_opener_api.ScreenOpenerAPI;
import com.paragon_software.native_engine.EngineArticleAPI;
import com.paragon_software.sound_manager.SoundManagerAPI;

import java.util.List;

import io.reactivex.Completable;
import io.reactivex.Observable;

public abstract class WotDManagerAPI
{

  public static final String WOTD_ITEM_EXTRA = "WOTD_ITEM_EXTRA";

  public interface Factory
  {
    WotDManagerAPI create();

    Factory registerContext( Context context );

    Factory registerSettingsManager( SettingsManagerAPI settingManager );

    Factory registerNotificationOptions( @NonNull Class<? extends Activity> _activity,
                                         @DrawableRes int _smallIconResId,
                                         @NonNull Bitmap _largeIcon );

    Factory registerResProvider(WotDResProviderAPI resProvider );

    Factory registerEngine(EngineArticleAPI articleEngine);

    Factory registerScreenOpener(ScreenOpenerAPI screenOpener);

    Factory registerSoundManager(SoundManagerAPI soundManager);

    Factory registerHintManager( @Nullable HintManagerAPI hintManager );

    Factory registerFileProviderAuthorities(String authoritiesFileProvider);

    Factory registerDictionaryManager(DictionaryManagerAPI dictionaryManager);
  }

  abstract Observable<Boolean> isUpdating();

  abstract Observable< List< WotDItem > > getItems();

  abstract Observable< WotDItem > getCurrentItem();

  public abstract Observable< Integer > getNewWotDCount();

  abstract Completable updateWotDList();

  public abstract void openWotDItemScreen( WotDItem item );

  abstract WotDNotificationOptions getNotificationOptions();

  public abstract boolean isReceiveWotdSettingEnabled();

  abstract void registerShowPushNotificationCommand();

  abstract String getString( WotDRes wotDRes  );

  abstract WotDControllerAPI getController( String controllerId );
}
