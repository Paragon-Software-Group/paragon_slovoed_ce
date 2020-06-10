package com.paragon_software.article_manager;

import android.content.Context;
import android.graphics.drawable.Drawable;
import androidx.annotation.IdRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager_ui.R;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

final class ArticleButtons extends BaseArticleButtons
{
  //  public static final int[] IDS   = {
  //      R.id.article_manager_ui_find_next, R.id.article_manager_ui_find_previous, R.id.article_manager_ui_play_sound,
  //      R.id.article_manager_ui_toggle_hide_or_switch_blocks_state, R.id.article_manager_ui_search_in_article,
  //      R.id.article_manager_ui_add_to_favorites };
  //  public static final int   COUNT = IDS.length;

  //  public static @Nullable
  //  ArticleButtonState getInitialState( Context ctx, @IdRes int id )
  //  {
  //    ArticleButtonState res = null;
  //    ButtonState state = getInitialState(id);
  //    if ( state != null )
  //    {
  //      res = get(ctx, id, state);
  //    }
  //    return res;
  //  }

  @Nullable ArticleButtonState getBtnState( Context ctx, @IdRes int id, @NonNull ButtonState state )
  {
    ArticleButtonState res = null;
    Drawable icon = getIcon(ctx, id, state);
    String text = getText(ctx, id, state);
    if ( ( icon != null ) && ( text != null ) )
    {
      res = new ArticleButtonState(state.getVisibility(), state.getCheckState(), icon, text);
    }
    return res;
  }

  //  @Nullable
  //  private static ButtonState getInitialState( @IdRes int id )
  //  {
  //    ButtonState res = null;
  //    if ( haveId(id) )
  //    {
  //      boolean unchecked = ( ( id == R.id.article_manager_ui_add_to_favorites ) || ( id
  //          == R.id.article_manager_ui_toggle_hide_or_switch_blocks_state ) );
  //      res = new ButtonState(VisibilityState.enabled, unchecked ? CheckState.unchecked : CheckState.uncheckable);
  //    }
  //    return res;
  //  }

  //  private static boolean haveId( @IdRes int id )
  //  {
  //    int i;
  //    for ( i = 0; i < COUNT ; i++ )
  //    {
  //      if ( id == IDS[i] )
  //      {
  //        break;
  //      }
  //    }
  //    return i < COUNT;
  //  }

  @Nullable
  private static Drawable getIcon( Context ctx, @IdRes int id, @NonNull ButtonState state )
  {
    int iconResId = 0;
    if ( id == R.id.share_go_to_dictionary )
    {
      iconResId = R.drawable.icn_share_go_to_app_dark;
    }
    else if ( id == R.id.article_manager_ui_find_next )
    {
      iconResId = R.drawable.ic_keyboard_arrow_down_24px;
    }
    else if ( id == R.id.article_manager_ui_find_previous )
    {
      iconResId = R.drawable.ic_keyboard_arrow_up_24px;
    }
    else if ( id == R.id.article_manager_ui_play_sound )
    {
      if ( state.getCheckState() != CheckState.checked )
      {
        iconResId = R.drawable.icn_sound;
      }
      else
      {
        iconResId = R.drawable.icn_sound_checked;
      }
    }
    else if ( id == R.id.article_manager_ui_toggle_hide_or_switch_blocks_state )
    {
      if ( state.getVisibility() == VisibilityState.disabled )
      {
        iconResId = R.drawable.icn_showpassword_black_disabled;
      }
      else if ( state.getCheckState() == CheckState.checked )
      {
        iconResId = R.drawable.icn_hidepassword_black;
      }
      else
      {
        iconResId = R.drawable.icn_showpassword_black;
      }
    }
    else if ( id == R.id.article_manager_ui_search_in_article )
    {
      if ( state.getCheckState() != CheckState.checked )
      {
        iconResId = R.drawable.search;
      }
      else
      {
        iconResId = R.drawable.ic_clear_24px;
      }
    }
    else if ( id == R.id.article_manager_ui_add_to_favorites )
    {
      if ( state.getCheckState() != CheckState.checked )
      {
        iconResId = R.drawable.icn_favorites_greystroke;
      }
      else
      {
        iconResId = R.drawable.icn_favorites_grey;
      }
    }
    else if ( id == R.id.article_manager_ui_add_to_flashcards )
    {
      if ( state.getCheckState() != CheckState.checked )
      {
        iconResId = R.drawable.icn_flashcards_greystroke;
      }
      else
      {
        iconResId = R.drawable.icn_flashcards_grey;
      }
    }
    Drawable res = null;
    if ( iconResId != 0 )
    {
      res = ctx.getResources().getDrawable(iconResId);
    }
    return res;
  }

  @Nullable
  private static String getText( Context ctx, @IdRes int id, @NonNull ButtonState state )
  {
    int strResId = 0;
    if ( id == R.id.share_go_to_dictionary )
    {
      strResId = R.string.utils_slovoed_ui_common_share_btn_go_to_dictionary;
    }
    else if ( id == R.id.article_manager_ui_find_next )
    {
      strResId = R.string.article_manager_ui_find_next;
    }
    else if ( id == R.id.article_manager_ui_find_previous )
    {
      strResId = R.string.article_manager_ui_find_previous;
    }
    else if ( id == R.id.article_manager_ui_play_sound )
    {
      strResId = R.string.article_manager_ui_play_sound;
    }
    else if ( id == R.id.article_manager_ui_toggle_hide_or_switch_blocks_state )
    {
      if ( state.getCheckState() != CheckState.checked )
      {
        strResId = R.string.article_manager_ui_show_hide_or_switch_blocks;
      }
      else
      {
        strResId = R.string.article_manager_ui_hide_hide_or_switch_blocks;
      }
    }
    else if ( id == R.id.article_manager_ui_search_in_article )
    {
      if ( state.getCheckState() != CheckState.checked )
      {
        strResId = R.string.article_manager_ui_search_in_article_hint;
      }
      else
      {
        strResId = R.string.article_manager_ui_close_search_interface;
      }
    }
    else if ( id == R.id.article_manager_ui_add_to_favorites )
    {
      if ( state.getCheckState() != CheckState.checked )
      {
        strResId = R.string.article_manager_ui_add_to_favorites_hint;
      }
      else
      {
        strResId = R.string.article_manager_ui_remove_from_favorites_hint;
      }
    }
    else if ( id == R.id.article_manager_ui_add_to_flashcards )
    {
      if ( state.getCheckState() != CheckState.checked )
      {
        strResId = R.string.utils_slovoed_ui_add_to_flashcards_hint;
      }
      else
      {
        strResId = R.string.article_manager_ui_remove_from_flashcards_hint;
      }
    }
    String res = null;
    if ( strResId != 0 )
    {
      res = ctx.getString(strResId);
    }
    return res;
  }
}
