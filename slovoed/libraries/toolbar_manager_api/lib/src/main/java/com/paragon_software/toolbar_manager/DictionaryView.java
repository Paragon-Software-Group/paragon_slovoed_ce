package com.paragon_software.toolbar_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary.DictionaryId;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.utils_slovoed.text.LocalizedString;

import java.util.Collection;

public class DictionaryView
{
  private final DictionaryId              mId;
  private final LocalizedString           mTitle;
  private final IDictionaryIcon           mIcon;
  private final Collection<DirectionView> mDirectionViews;

  public DictionaryView( @Nullable DictionaryId id, @NonNull LocalizedString title, @NonNull IDictionaryIcon icon, Collection<DirectionView> directionViews ){
    mId = id;
    mTitle = title;
    mIcon = icon;
    mDirectionViews = directionViews;
  }

  @Nullable
  public DictionaryId getId()
  {
    return mId;
  }

  @NonNull
  public LocalizedString getTitle()
  {
    return mTitle;
  }

  @NonNull
  public IDictionaryIcon getIcon()
  {
    return mIcon;
  }

  public Collection< DirectionView > getDirectionViews()
  {
    return mDirectionViews;
  }

}
