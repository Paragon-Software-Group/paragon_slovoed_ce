package com.paragon_software.hint_manager;

import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;

public abstract class HintUIHelperAPI
{
  public abstract boolean showHintDialog( @Nullable HintType hintType, @Nullable FragmentManager fragmentManager );
}
