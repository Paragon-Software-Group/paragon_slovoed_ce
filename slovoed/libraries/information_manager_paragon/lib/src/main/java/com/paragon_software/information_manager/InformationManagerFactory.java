package com.paragon_software.information_manager;

import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.about_manager.AboutManagerAPI;
import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;

public class InformationManagerFactory implements InformationManagerFactoryAPI
{
  @NonNull
  @Override
  public InformationManagerAPI create( @NonNull DictionaryManagerAPI dictionaryManager,
                                       @NonNull SearchEngineAPI searchEngine, @NonNull AboutManagerAPI aboutManager,
                                       @NonNull String aboutControllerName, @NonNull ArticleManagerAPI articleManager,
                                       @NonNull String articleControllerName, @Nullable Intent privacyPolicyIntent,
                                       @Nullable Intent eulaIntent, @Nullable Intent rateAppIntent )
  {
    return new InformationManagerParagon(dictionaryManager, searchEngine, aboutManager, aboutControllerName,
                                         articleManager, articleControllerName, privacyPolicyIntent, eulaIntent, rateAppIntent);
  }
}
