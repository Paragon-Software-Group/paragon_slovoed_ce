
/*
 * local dictionary library exceptions
 *
 *  Created on: 04.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.local_dictionary_library.exceptions;

/**
 * Base class for Exceptions, that can be thrown from local dictionary library
 */
@SuppressWarnings( { "WeakerAccess", "unused" } )
public class LocalDictionaryLibraryException extends Exception
{
  public LocalDictionaryLibraryException()
  {
    super();
  }

  public LocalDictionaryLibraryException( Throwable cause )
  {
    super(cause);
  }

  public LocalDictionaryLibraryException( String message )
  {
    super(message);
  }
}
