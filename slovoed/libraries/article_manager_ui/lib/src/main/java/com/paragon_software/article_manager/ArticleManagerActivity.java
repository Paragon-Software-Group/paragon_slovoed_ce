package com.paragon_software.article_manager;

import android.os.Bundle;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import android.text.Html;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;

import com.paragon_software.article_manager_ui.R;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

public class ArticleManagerActivity extends BaseArticleActivity
{

  @Override
  public void onCreate( @Nullable Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_base_article_manager);

    Bundle b = getIntent().getExtras();
    String controllerId = null;
    if ( b != null )
    {
      ArticleManagerAPI articleManager = ArticleManagerHolder.getManager();
      controllerId = b.getString(ArticleManagerAPI.EXTRA_CONTROLLER_ID);
      if ( ( articleManager != null ) && ( controllerId != null ) )
      {
        setArticleController(articleManager.getArticleController(controllerId));
        getArticleController().setActive();
      }
    }

    if ( savedInstanceState == null )
    {
      ArticleFragment articleFragment = new ArticleFragment();
      if ( controllerId != null )
      {
        b = new Bundle(1);
        b.putString(ArticleManagerAPI.EXTRA_CONTROLLER_ID, controllerId);
        articleFragment.setArguments(getIntent().getExtras());
      }
      getSupportFragmentManager().beginTransaction().add(R.id.article_fragment, articleFragment).commit();
    }

    ActionBar actionBar = getSupportActionBar();
    if ( actionBar != null )
    {
      actionBar.setDisplayShowTitleEnabled(false);
      actionBar.setDisplayHomeAsUpEnabled(true);

      SearchInArticleInputField searchInArticleInputField =
          (SearchInArticleInputField) getLayoutInflater().inflate(R.layout.search_in_artcle_input_field, null);
      if ( getArticleController() != null )
      {
        searchInArticleInputField.setSearchText(getArticleController().getSearchUIInitialText());
      }
      searchInArticleInputField.<EditText>findViewById(searchInArticleInputField.getEditTextInputId()).setHint(Html.fromHtml(getString(R.string.article_manager_ui_find_on_page_edit_text_hint)));
      actionBar.setCustomView(searchInArticleInputField);
      actionBar.setDisplayShowCustomEnabled(false);
    }
  }

  @Override
  protected void onDestroy()
  {
    Bundle b = getIntent().getExtras();
    String controllerId;
    if ( b != null )
    {
      ArticleManagerAPI articleManager = ArticleManagerHolder.getManager();
      controllerId = b.getString(ArticleManagerAPI.EXTRA_CONTROLLER_ID);
      if ( ( articleManager != null ) && ( controllerId != null ) )
      {
        articleManager.freeArticleController(controllerId);
        setArticleController(null);
      }
    }
    super.onDestroy();
  }

  @Override
  public boolean onCreateOptionsMenu( Menu menu )
  {
    for ( int id : BUTTON_IDS )
    {
      menu.add(Menu.NONE, id, Menu.NONE, "").setVisible(false).setEnabled(false)
          .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
    }
    return true;
  }

  @Override
  public void onSearchUIVisibilityChanged()
  {
    supportInvalidateOptionsMenu();
    ActionBar actionBar = getSupportActionBar();
    if ( ( actionBar != null ) && ( getArticleController() != null ) )
    {
      boolean show = !getArticleController().getSearchUIVisibility().equals(VisibilityState.gone);
      actionBar.setDisplayShowCustomEnabled(show);
      if ( show )
      {
        BaseSearchInArticleInputField searchField = getSearchField(actionBar);
        if ( searchField != null )
        {
          searchField.requestFocus();
        }
      }
    }
  }

  @Override
  @Nullable
  protected BaseSearchInArticleInputField getSearchField(@Nullable ActionBar actionBar )
  {
    BaseSearchInArticleInputField res = null;
    if ( actionBar == null )
    {
      actionBar = getSupportActionBar();
    }
    if ( actionBar != null )
    {
      View view = actionBar.getCustomView();
      if ( view instanceof BaseSearchInArticleInputField)
      {
        res = (BaseSearchInArticleInputField) view;
      }
    }
    return res;
  }

  @Nullable
  @Override
  protected BaseArticleButtons getArticleButtons() {
    return new ArticleButtons();
  }
}
