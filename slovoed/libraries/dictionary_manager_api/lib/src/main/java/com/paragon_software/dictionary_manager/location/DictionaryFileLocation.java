package com.paragon_software.dictionary_manager.location;

import java.io.File;

/**
 * Class to store dictionary location in a separate file.
 */
// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public class DictionaryFileLocation implements DictionaryLocation
{
  /**
   * The name of dictionary file.
   */
  private final File mFile;

  public DictionaryFileLocation( File file )
  {
    mFile = file;
  }

  public File getFile()
  {
    return mFile;
  }

  @Override
  public boolean equals( Object obj )
  {
    if ( obj == this )
    {
      return true;
    }
    if ( obj == null )
    {
      return false;
    }
    if ( getClass() != obj.getClass() )
    {
      return false;
    }
    DictionaryFileLocation other = (DictionaryFileLocation) obj;
    return other.mFile.getAbsolutePath().equals(this.mFile.getAbsolutePath());
  }

  @Override
  public int hashCode()
  {
    return mFile.getAbsolutePath().hashCode();
  }

  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "file: " + mFile.getAbsolutePath();
  }
}
