/*
 * dictionary manager api
 *
 *  Created on: 03.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.exceptions;

/**
 * Throws when dictionary download failed. Check reason for get detailed info
 */
@SuppressWarnings( "unused" )
public class DownloadDictionaryException extends DictionaryManagerException
{
  public DownloadDictionaryException()
  {
  }

  public DownloadDictionaryException( Throwable cause )
  {
    super(cause);
  }

  public DownloadDictionaryException( String message )
  {
    super(message);
  }
}
