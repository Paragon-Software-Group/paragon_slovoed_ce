package com.paragon_software.dictionary_manager;

import android.graphics.Bitmap;

import com.paragon_software.dictionary_manager.exceptions.NotALocalResourceException;

import androidx.annotation.Nullable;

public abstract class MockDictionaryIcon implements IDictionaryIcon {
  private final int id;
  private final Bitmap bitmap;

  MockDictionaryIcon(int _id, Bitmap _bitmap) {
    id = _id;
    bitmap = _bitmap;
  }

  @Nullable
  @Override
  public Bitmap getBitmap() {
    return bitmap;
  }

  @Override
  public int getResourceId() throws NotALocalResourceException {
    return id;
  }

  @Override
  public boolean equals(Object obj) {
    boolean res = false;
    if (obj instanceof MockDictionaryIcon) {
      try {
        res = (getResourceId() == ((MockDictionaryIcon) obj).getResourceId());
      } catch (NotALocalResourceException ignore) {

      }
    }
    return res;
  }
}