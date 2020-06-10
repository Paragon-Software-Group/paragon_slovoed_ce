/*
 * download manager
 *
 *  Created on: 03.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.download_manager;

import com.paragon_software.dictionary_manager.download_manager.exceptions.WriteToDestinationException;
import com.paragon_software.dictionary_manager.local_dictionary_library.exceptions.WriteToDictionaryLibraryException;

public interface IDownloadDestination
{
  void create() throws WriteToDestinationException;

  void write( final byte[] data, final int size ) throws WriteToDestinationException;

  void remove() throws WriteToDestinationException;

  void finish() throws WriteToDestinationException;
}
