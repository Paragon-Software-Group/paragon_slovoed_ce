package com.paragon_software.utils_slovoed.json;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.google.gson.stream.JsonReader;

public final class JsonUtils
{

  private JsonUtils() { }

  @SuppressWarnings( "SameParameterValue" )
  public static @Nullable <T> T parseRawJsonFile( @NonNull Context context, String fileName, @NonNull TypeToken<T> typeToken )
  {
    T result = null;
    String packageName = context.getPackageName();
    int fileResId = context.getResources().getIdentifier(fileName, "raw", packageName);
    if ( fileResId != 0 )
    {
      InputStream fileInputStream = context.getResources().openRawResource(fileResId);
      JsonReader fileJsonReader = new JsonReader(new InputStreamReader(fileInputStream));
      result = new Gson().fromJson(fileJsonReader, typeToken.getType());
      try
      {
        fileJsonReader.close();
      }
      catch ( IOException exception )
      {
        exception.printStackTrace();
      }
    }
    return result;
  }
}
