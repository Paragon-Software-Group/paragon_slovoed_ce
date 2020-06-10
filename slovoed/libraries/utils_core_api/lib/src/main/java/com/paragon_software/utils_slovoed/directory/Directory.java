package com.paragon_software.utils_slovoed.directory;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public abstract class Directory< ITEM_TYPE >
{
  @Nullable
  private final Directory< ITEM_TYPE >         mParent;
  @NonNull
  protected     List< Directory< ITEM_TYPE > > mChildList = new ArrayList<>();
  @NonNull
  protected     List< ITEM_TYPE >              mItems     = new ArrayList<>();
  @NonNull
  private final String                         mName;

  protected Directory( @Nullable Directory <ITEM_TYPE>  parent, @NonNull String name )
  {
    this.mParent = parent;
    this.mName = name;
  }

  @NonNull
  public final String getName()
  {
    return mName;
  }

  @Nullable
  public final Directory< ITEM_TYPE > getParent()
  {
    return mParent;
  }

  @NonNull
  public List< Directory< ITEM_TYPE > > getChildList()
  {
    return Collections.unmodifiableList(mChildList);
  }

  @NonNull
  public List< ITEM_TYPE > getItems()
  {
    return Collections.unmodifiableList(mItems);
  }

  @NonNull
  public String getPath()
  {
    return mParent != null ? mParent.getPath() + mName : mName;
  }

  @Nullable
  public Directory< ITEM_TYPE > findDirectory( @NonNull String pathToFind )
  {
    if ( pathToFind.equals(this.getPath()) )
    {
      return this;
    }
    else
    {
      if ( getParent() != null && pathToFind.equals(getParent().getPath()) )
      {
        return getParent();
      }
      for ( Directory childDir : mChildList )
      {
        Directory res = childDir.findDirectory(pathToFind);
        if ( res != null )
        {
          return res;
        }
      }
      return null;
    }
  }

  public boolean hasItem( @Nullable ITEM_TYPE item )
  {
    return mItems.contains(item);
  }

  public int depth()
  {
    return ( getParent() != null ) ? ( getParent().depth() + 1 ) : 0;
  }

  @Override
  public boolean equals( Object obj )
  {
    if ( this == obj )
    {
      return true;
    }
    if ( obj == null || getClass() != obj.getClass() )
    {
      return false;
    }
    Directory that = (Directory) obj;
    return that.toString().equals(this.toString());
  }

  @Override
  public String toString()
  {
    return "Directory{" + "Parent=" + ( mParent == null ? "" : mParent.getPath() ) + ", ChildList="
        + mChildList + ", Items=" + mItems + ", Name='" + mName + "'}";
  }
}

