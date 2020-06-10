package com.paragon_software.favorites_manager;

import androidx.annotation.NonNull;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import com.google.gson.annotations.SerializedName;
import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.utils_slovoed.directory.Directory;

class PersistentDirectory implements Serializable
{

  // WARNING!!! Don't change @SerializedName values. They already
  // was used in persistent storage of real users devices.
  // (it has so strange names because of ProGuard)
  @SerializedName("a")
  @NonNull
  private List< PersistentDirectory > mChildList = new ArrayList<>();
  @SerializedName("b")
  @NonNull
  private String                      mName;

  PersistentDirectory( @NonNull Directory< ArticleItem > directory )
  {
    for ( Directory< ArticleItem > dir : directory.getChildList() )
    {
      this.mChildList.add(new PersistentDirectory(dir));
    }
    this.mName = directory.getName();
  }

  @NonNull
  List< PersistentDirectory > getChildList()
  {
    return mChildList;
  }

  @NonNull
  String getName()
  {
    return mName;
  }

}
