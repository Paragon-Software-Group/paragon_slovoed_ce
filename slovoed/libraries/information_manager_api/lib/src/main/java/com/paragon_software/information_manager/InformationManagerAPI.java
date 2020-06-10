package com.paragon_software.information_manager;

import android.content.Context;
import androidx.annotation.NonNull;

public abstract class InformationManagerAPI
{
  @NonNull
  public abstract InformationControllerAPI getController(@NonNull String controllerName);

  abstract void openInformationItem( @NonNull Context context, @NonNull InformationItem item );
}
