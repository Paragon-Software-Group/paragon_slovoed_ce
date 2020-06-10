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
 * Base class for Exceptions, that can be thrown from download manager
 */
@SuppressWarnings( "WeakerAccess" )

public class DownloadManagerException extends Exception
{
  public DownloadManagerException()
  {
    super();
  }

  public DownloadManagerException( Throwable cause )
  {
    super(cause);
  }

  public DownloadManagerException( String message )
  {
    super(message);
  }
}
