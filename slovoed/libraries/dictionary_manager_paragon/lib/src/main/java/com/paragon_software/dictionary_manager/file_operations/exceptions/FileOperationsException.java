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

/**
 * Base class for Exceptions, that can be thrown from file operations
 */
@SuppressWarnings( { "WeakerAccess", "unused" } )

public class FileOperationsException extends Exception
{
  public FileOperationsException()
  {
    super();
  }

  public FileOperationsException( Throwable cause )
  {
    super(cause);
  }

  public FileOperationsException( String message )
  {
    super(message);
  }
}
