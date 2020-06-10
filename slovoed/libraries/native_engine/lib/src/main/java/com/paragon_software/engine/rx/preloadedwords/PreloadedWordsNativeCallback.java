package com.paragon_software.engine.rx.preloadedwords;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.List;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.engine.nativewrapper.ArticleItemFactory;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.utils_slovoed.directory.Directory;

public class PreloadedWordsNativeCallback
{
  @NonNull
  private Dictionary mDictionary;
  @NonNull
  private NativeDictionary mNativeDictionary;
  @NonNull
  private MutableDirectory mRootDirectory;
  @NonNull
  private MutableDirectory mCurDirectory;

  public PreloadedWordsNativeCallback( @NonNull Dictionary dictionary, @NonNull NativeDictionary nativeDictionary )
  {
    mDictionary = dictionary;
    mNativeDictionary= nativeDictionary;

    mRootDirectory = new MutableDirectory(null, "root");
    mCurDirectory = mRootDirectory;
  }

  // used in native functions
  public void addNewChildDirectory( String name )
  {
    // add new child dir and navigate inside it
    MutableDirectory newDir = new MutableDirectory(mCurDirectory, name);
    mCurDirectory.getMutableChildList().add(newDir);
    mCurDirectory = newDir;
  }

  // used in native functions
  public void navigateToUpDirectory()
  {
    // navigate to parent directory
    if ( mCurDirectory.getParent() instanceof MutableDirectory )
    {
      mCurDirectory = (MutableDirectory) mCurDirectory.getParent();
    }
  }

  // used in native functions
  public void addArticleItem( int listIndex, int globalIndex )
  {
    ArticleItem newItem = ArticleItemFactory.createNormal(mDictionary.getId(), mNativeDictionary, listIndex, globalIndex );
    mCurDirectory.getMutableItems().add(newItem);
  }

  public Directory< ArticleItem > getRootDirectory()
  {
    return mRootDirectory;
  }

  private class MutableDirectory extends Directory< ArticleItem >
  {

    MutableDirectory( @Nullable Directory parent, @NonNull String name )
    {
      super(parent, name);
    }

    List< Directory< ArticleItem > > getMutableChildList()
    {
      return mChildList;
    }

    List< ArticleItem > getMutableItems()
    {
      return mItems;
    }
  }
}
