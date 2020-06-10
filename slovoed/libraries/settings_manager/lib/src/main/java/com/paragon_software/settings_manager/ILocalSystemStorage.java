package com.paragon_software.settings_manager;

import androidx.annotation.NonNull;

import java.util.HashMap;

import com.paragon_software.settings_manager.exceptions.LocalResourceUnavailableException;

/*
 * interface for local system storage
 *
 *  Created on: 14.02.18 with love
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

public interface ILocalSystemStorage
{

  void save( @NonNull final String key, @NonNull final String value ) throws LocalResourceUnavailableException;

  @NonNull
  HashMap< String, String > load() throws LocalResourceUnavailableException;

  void clear() throws LocalResourceUnavailableException;
}
