/*
 * file operations exceptions
 *
 *  Created on: 05.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.file_operations.exceptions;

@SuppressWarnings( "unused" )
public class CantCreateFileException extends FileOperationsException
{
  public CantCreateFileException( String message )
  {
    super(message);
  }

  public CantCreateFileException( Throwable cause )
  {
    super(cause);
  }
}
