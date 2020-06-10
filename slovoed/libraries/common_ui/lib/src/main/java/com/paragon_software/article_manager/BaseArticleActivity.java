package com.paragon_software.article_manager;

import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import androidx.annotation.IdRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;

import com.paragon_software.common_ui.R;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

import java.lang.ref.WeakReference;

public abstract class BaseArticleActivity extends AppCompatActivity
    implements OnFavoritesButtonStateChangedListener, OnSoundButtonStateChangedListener,
        OnSearchUIVisibilityChangedListener, OnSearchUIResultTextChangedListener,
        OnFindNextButtonStateChangedListener, OnFindPreviousButtonStateChangedListener,
        OnSearchUIButtonStateChangedListener, OnHideOrSwitchBlocksButtonStateChangedListener,
        OnGoToHistoryButtonStateChangedListener, OnTextChangedListener, OnControllerErrorListener,
        OnFlashcardButtonStateChangedListener
{
  protected static final int[] BUTTON_IDS = {
      R.id.article_manager_ui_play_sound, R.id.article_manager_ui_toggle_hide_or_switch_blocks_state,
      R.id.article_manager_ui_find_next, R.id.article_manager_ui_find_previous,
      R.id.article_manager_ui_search_in_article, R.id.article_manager_ui_add_to_favorites,
      R.id.share_go_to_dictionary, R.id.article_manager_ui_add_to_flashcards };
  private ArticleControllerAPI mArticleController;

  @Override
  public boolean onPrepareOptionsMenu( Menu menu )
  {
    super.onPrepareOptionsMenu(menu);
    if ( ( getArticleController() != null ) && ( menu != null ) )
    {
      updateButton(menu, R.id.article_manager_ui_play_sound, getArticleController().getSoundButtonState());
      updateButton(menu, R.id.article_manager_ui_find_next, getArticleController().getFindNextButtonState());
      updateButton(menu, R.id.article_manager_ui_find_previous, getArticleController().getFindPreviousButtonState());
      updateButton(menu, R.id.article_manager_ui_search_in_article, getArticleController().getSearchUIButtonState());
      updateButton(menu, R.id.article_manager_ui_toggle_hide_or_switch_blocks_state,
                   getArticleController().getHideOrSwitchBlocksButtonState());
      updateButton(menu, R.id.article_manager_ui_add_to_favorites, getArticleController().getFavoritesButtonState());
      updateButton(menu, R.id.share_go_to_dictionary, new ButtonState(VisibilityState.gone, CheckState.uncheckable));
      updateButton(menu, R.id.article_manager_ui_pronunciation_practice, getArticleController().getSearchUIButtonState());
      updateButton(menu, R.id.article_manager_ui_go_to_history, getArticleController().getGoToHistoryButtonState());
      updateButton(menu, R.id.article_manager_ui_add_to_flashcards, getArticleController().getFlashcardButtonState());
      updateButton(menu, R.id.article_manager_ui_morphological_table, getArticleController().getMorphoTableButtonState());
    }
    return true;
  }

  @Override
  protected void onResume()
  {
    super.onResume();
    if ( getArticleController() != null )
    {
      getArticleController().registerNotifier(this);
      onFavoritesButtonStateChanged();
      onSoundButtonStateChanged();
      onSearchUIVisibilityChanged();
      onSearchUIResultTextChanged();
      onFindNextButtonStateChanged();
      onFindPreviousButtonStateChanged();
      onSearchUIButtonStateChanged();
      onHideOrSwitchBlocksButtonStateChanged();
      onFlashcardButtonStateChanged();
    }
    BaseSearchInArticleInputField searchField = getSearchField(null);
    if ( searchField != null )
    {
      searchField.addOnTextChangedListener(this);
    }
  }

  @Override
  protected void onPause()
  {
    super.onPause();
    BaseSearchInArticleInputField searchField = getSearchField(null);
    if ( searchField != null )
    {
      searchField.removeOnTextChangedListener(this);
    }
    if ( getArticleController() != null )
    {
      getArticleController().unregisterNotifier(this);
      getArticleController().saveState();
      if ( isFinishing() )
      {
        getArticleController().free();
      }
    }
  }

  @Override
  public boolean onOptionsItemSelected( MenuItem item )
  {
    boolean res = false;
    int id = item.getItemId();
    if ( id == android.R.id.home )
    {
      onBackPressed();
      res = true;
    }
    else if ( getArticleController() != null )
    {
      if ( id == R.id.article_manager_ui_add_to_favorites )
      {
        if ( !buttonStateToBoolean(getArticleController().getFavoritesButtonState()) )
        {
          getArticleController().addToFavorites();
        }
        else
        {
          getArticleController().removeFromFavorites();
        }
        res = true;
      }
      else if ( id == R.id.article_manager_ui_toggle_hide_or_switch_blocks_state )
      {
        getArticleController().toggleHideOrSwitchBlocksButtonCheckState(
            !buttonStateToBoolean(getArticleController().getHideOrSwitchBlocksButtonState()));
        res = true;
      }
      else if ( id == R.id.article_manager_ui_play_sound )
      {
        getArticleController().playCurrentSound();
        res = true;
      }
      else if ( id == R.id.article_manager_ui_search_in_article )
      {
        boolean show = !buttonStateToBoolean(getArticleController().getSearchUIButtonState());
        getArticleController().toggleSearchUI(show);
        if ( show )
        {
          BaseSearchInArticleInputField inputField = getSearchField(null);
          if ( inputField != null )
          {
            inputField.selectAll();
          }
        }
        else
        {
          hideKeyboard();
        }
        res = true;
      }
      else if ( id == R.id.article_manager_ui_find_next )
      {
        hideKeyboard();
        getArticleController().findNext();
        res = true;
      }
      else if ( id == R.id.article_manager_ui_find_previous )
      {
        hideKeyboard();
        getArticleController().findPrevious();
        res = true;
      }
      else if ( id == R.id.article_manager_ui_go_to_history )
      {
        getArticleController().showHistoryScreen(this);
        res = true;
      }
      else if ( id == R.id.article_manager_ui_add_to_flashcards )
      {
        if ( !buttonStateToBoolean(getArticleController().getFlashcardButtonState()) )
        {
          getArticleController().addToFlashcards(this);
        }
        else
        {
          getArticleController().removeFromFlashcards(this);
        }
        res = true;
      }
    }
    return res;
  }

  @Override
  public void onFavoritesButtonStateChanged()
  {
    supportInvalidateOptionsMenu();
  }

  @Override
  public void onFlashcardButtonStateChanged()
  {
    supportInvalidateOptionsMenu();
  }

  @Override
  public abstract void onSearchUIVisibilityChanged();

  @Override
  public void onSearchUIResultTextChanged()
  {
    supportInvalidateOptionsMenu();
    BaseSearchInArticleInputField searchField = getSearchField(null);
    if ( ( searchField != null ) && ( getArticleController() != null ) )
    {
      searchField.setResultText(getArticleController().getSearchUIResultText());
    }
  }

  @Override
  public void onFindNextButtonStateChanged()
  {
    supportInvalidateOptionsMenu();
  }

  @Override
  public void onFindPreviousButtonStateChanged()
  {
    supportInvalidateOptionsMenu();
  }

  @Override
  public void onSearchUIButtonStateChanged()
  {
    supportInvalidateOptionsMenu();
  }

  @Override
  public void onHideOrSwitchBlocksButtonStateChanged()
  {
    supportInvalidateOptionsMenu();
  }

  @Override
  public void onSoundButtonStateChanged()
  {
    supportInvalidateOptionsMenu();
  }

  @Override
  public void onGoToHistoryButtonStateChanged()
  {
    supportInvalidateOptionsMenu();
  }

  @Override
  public void onTextChanged( @NonNull String newText )
  {
    if ( getArticleController() != null )
    {
      getArticleController().search(newText);
    }
  }

  @Nullable
  protected abstract BaseArticleButtons getArticleButtons();

  private WeakReference<BaseArticleButtons> _baseArticleButtons;

  protected void updateButton( @NonNull Menu menu, @IdRes int id, @NonNull ButtonState buttonState ) {
    MenuItem menuItem = menu.findItem(id);
    if (_baseArticleButtons == null || _baseArticleButtons.get() == null) {
      _baseArticleButtons = new WeakReference<>(getArticleButtons());
    }
    if (_baseArticleButtons.get() == null) { return; }

    ArticleButtonState state = _baseArticleButtons.get().getBtnState(this, id, buttonState);
    if ( ( menuItem != null ) && ( state != null ) )
    {
      menuItem.setTitle(state.getText());
      menuItem.setIcon(state.getIcon());
      menuItem.setVisible(state.getVisibility() != VisibilityState.gone);
      menuItem.setEnabled(state.getVisibility() != VisibilityState.disabled);
    }
  }

  @SuppressWarnings( "BooleanMethodIsAlwaysInverted" )
  private boolean buttonStateToBoolean( @NonNull ButtonState buttonState )
  {
    return buttonState.getCheckState().equals(CheckState.checked);
  }

  @Nullable
  protected abstract BaseSearchInArticleInputField getSearchField(@Nullable ActionBar actionBar );

  protected void hideKeyboard()
  {
    KeyboardHelper.hideKeyboard(findViewById(R.id.article_fragment));
  }

  @Override
  public void onControllerError( Exception exception )
  {
    Toast.makeText(getApplicationContext(), exception.getMessage(), Toast.LENGTH_SHORT).show();
  }

  public ArticleControllerAPI getArticleController()
  {
    return mArticleController;
  }

  public void setArticleController( ArticleControllerAPI mController )
  {
    this.mArticleController = mController;
  }
}
