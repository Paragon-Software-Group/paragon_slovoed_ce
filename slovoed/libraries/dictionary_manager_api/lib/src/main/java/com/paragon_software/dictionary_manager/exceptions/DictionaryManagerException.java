/*
 * dictionary manager api
 *
 *  Created on: 19.03.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.exceptions;

/**
 * Base class for Exceptions, that can be thrown from dictionary manager API
 */
@SuppressWarnings( { "WeakerAccess", "unused" } )
public class DictionaryManagerException extends Exception
{
  public DictionaryManagerException()
  {
  }

  public DictionaryManagerException( Throwable cause )
  {
    super(cause);
  }

  public DictionaryManagerException( String message )
  {
    super(message);
  }
}
