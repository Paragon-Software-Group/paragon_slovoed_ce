/*
 * dictionary_manager_api
 *
 *  Created on: 23.03.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager;

import android.content.Context;
import androidx.annotation.NonNull;

@SuppressWarnings( "unused" )
public interface IDictionaryManagerFactory
{
  /**
   * Get dictionary list from it's id's
   *
   * @param context        application context
   * @param dictionariesId list of dictionaries id's
   *
   * @return list of dictionaries instances.
   */
  @NonNull
  Dictionary[] getDictionaries(@NonNull final Context context,
                               @NonNull final Dictionary.DictionaryId[] dictionariesId);

  @NonNull
  DictionaryPack[] getDictionaryPacks(@NonNull final Context context,
                                      @NonNull final Dictionary.DictionaryId[] dictionariesId);

}
