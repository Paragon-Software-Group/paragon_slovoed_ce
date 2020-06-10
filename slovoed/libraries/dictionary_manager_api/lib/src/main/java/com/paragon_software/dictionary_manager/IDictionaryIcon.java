/*
 * dictionary manager api
 *
 *  Created on: 19.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager;

import android.graphics.Bitmap;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.exceptions.NotALocalResourceException;

/*
 * interface class for dictionary icon. Icon may be placed on remote storage, or it can be a local resource.
 * NOTE Implementation of this interface should be immutable otherwise don't forget to add cloning method and use it in Dictionary objects cloning
 */
@SuppressWarnings( { "unused", "RedundantThrows" } )
public interface IDictionaryIcon
{
  /**
   * @return Image bitmap. null if cant get bitmap.
   */
  @Nullable
  Bitmap getBitmap();

  /**
   * @return image resource id
   *
   * @throws NotALocalResourceException if this class no represents a local resource.
   */
  int getResourceId() throws NotALocalResourceException;

}
