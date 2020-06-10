package com.paragon_software.information_manager;

import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.HashMap;
import java.util.Map;

import com.paragon_software.about_manager.AboutManagerAPI;
import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.article_manager.ArticleManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.SearchEngineAPI;

class InformationManagerParagon extends InformationManagerAPI
{
  @NonNull
  private final DictionaryManagerAPI dictionaryManager;
  @NonNull
  private final SearchEngineAPI searchEngine;
  @NonNull
  private final AboutManagerAPI   aboutManager;
  @NonNull
  private final String            aboutControllerName;
  @NonNull
  private final ArticleManagerAPI articleManager;
  @NonNull
  private final String            articleControllerName;
  @Nullable
  private final Intent            privacyPolicyIntent;
  @Nullable
  private final Intent            eulaIntent;
  @Nullable
  private Intent rateAppIntent;
  @NonNull
  private Map< String, InformationControllerAPI > controllers = new HashMap<>();

  InformationManagerParagon( @NonNull DictionaryManagerAPI dictionaryManager, @NonNull SearchEngineAPI searchEngine, @NonNull AboutManagerAPI aboutManager, @NonNull String aboutControllerName,
                             @NonNull ArticleManagerAPI articleManager, @NonNull String articleControllerName,
                             @Nullable Intent privacyPolicyIntent, @Nullable Intent eulaIntent, @Nullable Intent rateAppIntent)
  {
    this.dictionaryManager = dictionaryManager;
    this.searchEngine = searchEngine;
    this.aboutManager = aboutManager;
    this.aboutControllerName = aboutControllerName;
    this.articleManager = articleManager;
    this.articleControllerName = articleControllerName;
    this.privacyPolicyIntent = privacyPolicyIntent;
    this.eulaIntent = eulaIntent;
    this.rateAppIntent = rateAppIntent;
  }

  @NonNull
  @Override
  public InformationControllerAPI getController( @NonNull String controllerName )
  {
    InformationControllerAPI informationController = controllers.get(controllerName);
    if ( null == informationController )
    {
      informationController = new InformationController(this, dictionaryManager, searchEngine);
      controllers.put(controllerName, informationController);
    }
    return informationController;
  }

  @Override
  void openInformationItem( @NonNull Context context, @NonNull InformationItem item )
  {
    InformationItemType type = item.getType();
    switch ( type )
    {
      case EULA:
        if ( eulaIntent != null )
          context.startActivity(eulaIntent);
        break;
      case PRIVACY_POLICY:
        if ( privacyPolicyIntent != null )
          context.startActivity(privacyPolicyIntent);
        break;
      case ABOUT:
        aboutManager.showAbout(context, aboutControllerName, item.getDictionaryId());
        break;
      case RATE_APP:
        if ( rateAppIntent != null )
          context.startActivity(rateAppIntent);
        break;
      case ADDITIONAL_ARTICLES:
        ArticleItem articleItem = item.getArticleItem();
        if ( null != articleItem )
        {
          articleManager.showArticleActivity(articleItem, articleControllerName, context);
        }
        break;
    }
  }
}
