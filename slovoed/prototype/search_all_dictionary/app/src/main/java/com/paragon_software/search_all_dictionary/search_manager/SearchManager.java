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

import android.support.annotation.NonNull;

public abstract class SearchManager
{
  @NonNull
  abstract SearchController getController( @SuppressWarnings( "SameParameterValue" ) @NonNull final String name );
}
