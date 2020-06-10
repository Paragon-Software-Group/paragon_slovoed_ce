package com.paragon_software.information_manager;

import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.about_manager.AboutManagerAPI;
import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;

public interface InformationManagerFactoryAPI
{
  interface InformationScreenFactory
  {
    @Nullable Intent createIntent( @Nullable Context context );
  }

  @NonNull
  InformationManagerAPI create( @NonNull DictionaryManagerAPI dictionaryManager, @NonNull SearchEngineAPI searchEngine,
                                @NonNull AboutManagerAPI aboutManager, @NonNull String aboutControllerName,
                                @NonNull ArticleManagerAPI articleManager, @NonNull String articleControllerName,
                                @Nullable Intent privacyPolicyIntent, @Nullable Intent eulaIntent, @Nullable Intent rateAppIntent );
}
