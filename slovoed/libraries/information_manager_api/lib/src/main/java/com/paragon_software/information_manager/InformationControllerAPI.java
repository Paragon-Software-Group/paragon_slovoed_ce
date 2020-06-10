package com.paragon_software.information_manager;

import android.content.Context;
import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.Map;

public abstract class InformationControllerAPI
{
  @NonNull
  public abstract CollectionView< ? extends InformationItem, Void > getInformationItems(
      @NonNull Map< InformationItemType, String > types );

  public abstract void openInformationItem(@NonNull Context context, @NonNull InformationItem item );
}
