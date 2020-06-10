/*
 * SearchAllDictionary
 *
 *  Created on: 28.05.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.search_all_dictionary.search_manager;

import android.databinding.ObservableArrayList;
import android.databinding.ObservableField;
import android.support.annotation.NonNull;

public interface SearchController
{
  @NonNull
  ObservableField< String > getSearchText();

  @NonNull
  ObservableArrayList< SearchResult > getSearchResults();

  @NonNull
  ObservableField<Boolean> getOnlineSearch();

  void okInSoftKeyboard();
}
