/*
 * download manager
 *
 *  Created on: 04.04.18
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

public class NoSuchDownloadTaskException extends DownloadManagerException
{
  public NoSuchDownloadTaskException()
  {
  }
}
