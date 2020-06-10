package com.paragon_software.about_manager;

import android.app.Activity;
import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.EngineVersion;

import java.util.List;

public abstract class AboutManagerAPI
{
  static final String EXTRA_CONTROLLER_ID =
      AboutManagerAPI.class.getCanonicalName() + ".EXTRA_CONTROLLER_ID";

  public abstract void registerUI( @NonNull String controllerName, @NonNull Class< ? extends Activity > aClass );

  public abstract boolean showAbout( @NonNull Context context, @NonNull String controllerName, @Nullable Dictionary.DictionaryId dictionaryId );

  @NonNull
  abstract AboutControllerAPI getController( @NonNull String controllerName );

  @Nullable
  abstract Dictionary getDictionary( @NonNull Dictionary.DictionaryId dictionaryId );

  @NonNull
  abstract EngineVersion getEngineVersion();

  @NonNull
  abstract List<AboutSoundInfo> getSoundInfo( @NonNull Dictionary.DictionaryId dictionaryId );
}
