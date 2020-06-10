package com.paragon_software.engine.nativewrapper;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.SparseArray;

import com.paragon_software.native_engine.R;
import com.paragon_software.utils_slovoed.text.LocalizedString;
import com.paragon_software.utils_slovoed.text.LocalizedStringImpl;

import java.util.concurrent.atomic.AtomicReference;

class LocalizedStringBuilder implements NativeFunctions.Callback {
  private static final int ENGLISH = localeFromString("engl");
  private static final int DEFAULT = localeFromString("0000");
  private static final AtomicReference<Context> CONTEXT_REF = new AtomicReference<>(null);

  private final SparseArray< String > map = new SparseArray<>();

  LocalizedStringBuilder(Context context) {
    CONTEXT_REF.compareAndSet(null, context.getApplicationContext());
  }

  @NonNull
  @Override
  public Object function( Object... args ) {
    map.put((Integer) args[0], (String) args[1]);
    return 0;
  }

  @NonNull
  LocalizedString build() {
    return new LocalizedStringImpl<Integer>() {
      @Nullable
      @Override
      protected Integer getCurrentLocale() {
        return localeFromString(CONTEXT_REF.get().getString(R.string.native_engine_api_language));
      }

      @Nullable
      @Override
      protected Integer getDefaultLocale() {
        return DEFAULT;
      }

      @Nullable
      @Override
      protected Integer getEnglishLocale() {
        return ENGLISH;
      }

      @Nullable
      @Override
      protected String getStringByKey(@NonNull Integer key) {
        return map.get(key);
      }

      @Nullable
      @Override
      protected String getAnyString() {
        String res = null;
        if (map.size() > 0)
          res = map.valueAt(0);
        return res;
      }
    };
  }

  private static int localeFromString(@NonNull String locale) {
    int b1 = locale.codePointAt(3);
    int b2 = locale.codePointAt(2);
    int b3 = locale.codePointAt(1);
    int b4 = locale.codePointAt(0);
    return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
  }
}
