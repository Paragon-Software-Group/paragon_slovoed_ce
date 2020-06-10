/*
 * download manager
 *
 *  Created on: 03.04.18
 *      Author: Mikhail Belyshov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager.download_manager;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

import com.paragon_software.dictionary_manager.download_manager.exceptions.BadUrlException;
import com.paragon_software.dictionary_manager.download_manager.exceptions.DownloadFailedException;

@SuppressWarnings( "unused" )
public final class HttpUrlDownloadSource implements IDownloadSource
{
  private static final int DEFAULT_READ_TIMEOUT    = 10_000;
  private static final int DEFAULT_CONNECT_TIMEOUT = 10_000;
  private final static int BUFFER_SIZE = 4096 * 16;
  @NonNull
  private final URL                 mUrl;
  private final String              mUserAgent;
  private       HttpURLConnection   mUrlConnection;
  private       BufferedInputStream mBis;

  @VisibleForTesting
  HttpUrlDownloadSource( @NonNull final URL url, @NonNull final HttpURLConnection connection )
      throws DownloadFailedException
  {
    this.mUrl = url;
    this.mUrlConnection = connection;
    this.mBis = initBis(mUrlConnection, BUFFER_SIZE);
    this.mUserAgent = null;
  }

  public HttpUrlDownloadSource( @NonNull final String url, String userAgent ) throws BadUrlException
  {
    this.mUserAgent = userAgent;
    try
    {
      this.mUrl = new URL(url);
    }
    catch ( MalformedURLException e )
    {
      throw new BadUrlException(e);
    }
  }

  @Override
  public long length()
  {
    return mUrlConnection.getContentLength();
  }

  @Override
  public int read( @NonNull byte[] data ) throws DownloadFailedException
  {
    try
    {
      return mBis.read(data);
    }
    catch ( IOException e )
    {
      throw new DownloadFailedException(e);
    }
  }

  @Override
  public void finish()
  {
    closeConnection();
  }

  @Override
  public void open() throws DownloadFailedException
  {
    this.mUrlConnection = openConnection(mUrl);
    this.mBis = initBis(mUrlConnection, BUFFER_SIZE);
  }

  private void closeConnection()
  {
    if (null != mUrlConnection)
      mUrlConnection.disconnect();
  }

  private BufferedInputStream initBis( @NonNull final HttpURLConnection connection, final int bufferSize )
      throws DownloadFailedException
  {
    try
    {
      return new BufferedInputStream(mUrlConnection.getInputStream(), BUFFER_SIZE);
    }
    catch ( IOException e )
    {
      throw new DownloadFailedException(e);
    }
  }

  private HttpURLConnection openConnection( @NonNull final URL url ) throws DownloadFailedException
  {
    try
    {
      HttpURLConnection connection = (HttpURLConnection) url.openConnection();
      addUserAgent(connection);
      connection.setReadTimeout(DEFAULT_READ_TIMEOUT);
      connection.setConnectTimeout(DEFAULT_CONNECT_TIMEOUT);
      connection.connect();
      // expect HTTP 200 OK, so we don't mistakenly save error report
      // instead of the file
      final int responseCode = connection.getResponseCode();
      if ( HttpURLConnection.HTTP_OK != responseCode )
      {
        final String responseMessage = connection.getResponseMessage();
        connection.disconnect();
        throw new DownloadFailedException("Server returned HTTP " + responseCode + " " + responseMessage);
      }
      return connection;
    }
    catch ( IOException e )
    {
      throw new DownloadFailedException(e);
    }
  }

  private void addUserAgent( HttpURLConnection connection )
  {
    if ( null != mUserAgent )
    {
      connection.addRequestProperty(HttpHeaders.USER_AGENT_HEADER_NAME, mUserAgent);
    }
  }
}
