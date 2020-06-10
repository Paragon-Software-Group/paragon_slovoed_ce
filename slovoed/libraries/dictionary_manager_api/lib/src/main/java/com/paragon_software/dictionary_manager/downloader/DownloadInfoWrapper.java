package com.paragon_software.dictionary_manager.downloader;

import androidx.annotation.NonNull;

import java.net.URL;
import java.util.HashMap;

public class DownloadInfoWrapper {

  private URL url;
  private DownloadLibraryBuilder.DownloadStatus downloadStatus;
  private HashMap< String, Object > meta;
  private long currentBytes;
  private long fileSize;
  private long speed;

  public DownloadInfoWrapper( @NonNull URL url, @NonNull DownloadLibraryBuilder.DownloadStatus downloadStatus,
                              @NonNull HashMap< String, Object > meta, long currentBytes, long fileSize, long speed )
  {
    this.url = url;
    this.downloadStatus = downloadStatus;
    this.meta = meta;
    this.currentBytes = currentBytes;
    this.fileSize = fileSize;
    this.speed = speed;
  }

  public URL getUrl()
  {
    return url;
  }

  public DownloadLibraryBuilder.DownloadStatus getDownloadStatus()
  {
    return downloadStatus;
  }

  public HashMap< String, Object > getMeta()
  {
    return meta;
  }

  public long getCurrentBytes()
  {
    return currentBytes;
  }

  public long getFileSize()
  {
    return fileSize;
  }

  public long getSpeed()
  {
    return speed;
  }

  @Override
  public boolean equals( Object o )
  {
    if ( this == o )
    {
      return true;
    }
    if ( o == null || getClass() != o.getClass() )
    {
      return false;
    }

    DownloadInfoWrapper that = (DownloadInfoWrapper) o;

    if ( getCurrentBytes() != that.getCurrentBytes() )
    {
      return false;
    }
    if ( getFileSize() != that.getFileSize() )
    {
      return false;
    }
    if ( getSpeed() != that.getSpeed() )
    {
      return false;
    }
    if ( getUrl() != null ? !getUrl().equals(that.getUrl()) : that.getUrl() != null )
    {
      return false;
    }
    if ( getDownloadStatus() != that.getDownloadStatus() )
    {
      return false;
    }
    return getMeta() != null ? getMeta().equals(that.getMeta()) : that.getMeta() == null;
  }

  @Override
  public int hashCode()
  {
    int result = getUrl() != null ? getUrl().hashCode() : 0;
    result = 31 * result + ( getDownloadStatus() != null ? getDownloadStatus().hashCode() : 0 );
    result = 31 * result + ( getMeta() != null ? getMeta().hashCode() : 0 );
    result = 31 * result + (int) ( getCurrentBytes() ^ ( getCurrentBytes() >>> 32 ) );
    result = 31 * result + (int) ( getFileSize() ^ ( getFileSize() >>> 32 ) );
    result = 31 * result + (int) ( getSpeed() ^ ( getSpeed() >>> 32 ) );
    return result;
  }

  @Override
  public String toString()
  {
    return "DownloadInfoWrapper{" + "url=" + url + ", downloadStatus=" + downloadStatus + ", meta=" + meta
        + ", currentBytes=" + currentBytes + ", fileSize=" + fileSize + ", speed=" + speed + '}';
  }
}
