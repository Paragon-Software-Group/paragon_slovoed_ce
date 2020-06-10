/*
 * file operations
 *
 *  Created on: 06.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.file_operations;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.file_operations.exceptions.FileReadException;

public interface IFileStreamReader
{
  long length();

  int read( @NonNull final byte[] data, final int len ) throws FileReadException;

  void finish();
}
