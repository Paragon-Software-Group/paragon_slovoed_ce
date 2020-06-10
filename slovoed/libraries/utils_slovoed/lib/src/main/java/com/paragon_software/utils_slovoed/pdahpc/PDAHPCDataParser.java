package com.paragon_software.utils_slovoed.pdahpc;

import android.content.Context;
import android.content.res.Resources;
import androidx.annotation.Nullable;

import com.google.gson.reflect.TypeToken;
import com.paragon_software.utils_slovoed.json.JsonUtils;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

public final class PDAHPCDataParser
{

  private static Catalog catalog;
  private static Data xmlCatalog = null;

  private PDAHPCDataParser() { }

  public static @Nullable Catalog parseCatalog( Context context )
  {
    if ( catalog == null )
    {
      catalog = JsonUtils.parseRawJsonFile(context, "catalog", new TypeToken< Catalog >(){});
    }
    return catalog;
  }

  public static Data parseXmlCatalog(Context context) {
    if(xmlCatalog == null) {
      Resources resources = context.getResources();
      Serializer serializer = new Persister();
      int id = resources.getIdentifier("sou", "raw", context.getPackageName());
      try {
        xmlCatalog = serializer.read(Data.class, resources.openRawResource(id), false);
      } catch (Exception e) {
        throw new IllegalStateException("couldn't parse catalog", e);
      }
    }
    return xmlCatalog;
  }

  public static @Nullable About parseAbout( Context context )
  {
    return JsonUtils.parseRawJsonFile(context, "about", new TypeToken< About >(){});
  }

}
