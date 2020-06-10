/*
 * download manager
 *
 *  Created on: 03.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.download_manager.exceptions;

/**
 * Throw when there was an error in time to access to destination
 */

@SuppressWarnings( "unused" )
public class WriteToDestinationException extends DownloadManagerException
{

  public WriteToDestinationException()
  {
  }

  public WriteToDestinationException( Throwable cause )
  {
    super(cause);
  }
}
