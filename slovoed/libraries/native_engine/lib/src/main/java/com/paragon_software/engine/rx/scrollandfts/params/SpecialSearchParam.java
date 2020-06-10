package com.paragon_software.engine.rx.scrollandfts.params;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.SearchType;
import com.paragon_software.native_engine.SortType;

import java.util.Collection;

public class SpecialSearchParam extends AbstractParams
{
  @NonNull
  public final SearchType searchType;
  @NonNull
  public final SortType sortType;
  @NonNull
  public final Boolean needRunSearch;

  public SpecialSearchParam(@Nullable Dictionary.DictionaryId _dictionaryId, @Nullable Dictionary.Direction _direction,
                            @Nullable String _word, @Nullable Collection<Dictionary.Direction> _availableDirections,
                            @NonNull SearchType _searchType, @NonNull SortType _sortType, @NonNull Boolean _needRunSearch )
  {
    super(_dictionaryId, _direction, _word, _availableDirections);
    searchType = _searchType;
    sortType = _sortType;
    needRunSearch = _needRunSearch;
  }
}
