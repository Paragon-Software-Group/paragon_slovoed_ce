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

@SuppressWarnings( "unused" )
public class InitDictionaryLibraryException extends LocalDictionaryLibraryException
{
  public InitDictionaryLibraryException( String message )
  {
    super(message);
  }

  public InitDictionaryLibraryException( Throwable cause )
  {
    super(cause);
  }
}
