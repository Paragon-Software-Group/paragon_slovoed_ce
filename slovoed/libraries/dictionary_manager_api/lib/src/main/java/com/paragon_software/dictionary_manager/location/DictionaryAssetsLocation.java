package com.paragon_software.dictionary_manager.location;

/**
 * Class to store dictionary location in assets directory.
 */
// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public class DictionaryAssetsLocation implements DictionaryLocation
{
  /**
   * The name of dictionary file from the assets directory. This name can be hierarchical.
   */
  private final String mFileName;

  public DictionaryAssetsLocation( String fileName )
  {
    mFileName = fileName;
  }

  public String getFileName()
  {
    return mFileName;
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
    DictionaryAssetsLocation other = (DictionaryAssetsLocation) obj;
    return other.mFileName.equals(this.mFileName);
  }

  @Override
  public int hashCode()
  {
    return mFileName.hashCode();
  }

  @Override
  public String toString()
  {
    // NOTE When modify this method consider what it is used to create ParagonDictionaryManager object state trace
    return "file:///android_asset/" + mFileName;
  }
}
