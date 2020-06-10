package com.paragon_software.dictionary_manager.extbase;

import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.location.DictionaryLocation;

// NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
public abstract class ExternalBaseInfo
{
  protected int langFrom;

  protected @NonNull
  DictionaryLocation location;

  public ExternalBaseInfo( int langFrom, @NonNull DictionaryLocation location )
  {
    this.langFrom = langFrom;
    this.location = location;
  }

  public int getLangFrom()
  {
    return langFrom;
  }

  public DictionaryLocation getLocation()
  {
    return location;
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
    ExternalBaseInfo other = (ExternalBaseInfo) obj;
    return other.location.equals(this.location) && other.langFrom == this.langFrom;
  }

  @Override
  public int hashCode()
  {
    return location.hashCode() + this.langFrom;
  }
}
