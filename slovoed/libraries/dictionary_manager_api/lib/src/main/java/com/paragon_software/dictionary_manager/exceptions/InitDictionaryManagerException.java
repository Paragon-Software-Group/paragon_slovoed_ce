/*
 * dictionary manager api
 *
 *  Created on: 05.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.exceptions;

/**
 * Throws when dictionary initialization failed. Check reason for get detailed info
 */
@SuppressWarnings( "unused" )
public class InitDictionaryManagerException extends DictionaryManagerException
{
  public InitDictionaryManagerException()
  {
  }

  public InitDictionaryManagerException( Throwable cause )
  {
    super(cause);
  }

  public InitDictionaryManagerException( String message )
  {
    super(message);
  }
}
