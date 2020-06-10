/*
 * download manager
 *
 *  Created on: 02.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.download_manager.exceptions;

/**
 * Throw when bad URL was detected
 */
@SuppressWarnings( "WeakerAccess" )

public class BadUrlException extends DownloadManagerException
{
  public BadUrlException()
  {
  }

  public BadUrlException( Throwable cause )
  {
    super(cause);
  }
}
