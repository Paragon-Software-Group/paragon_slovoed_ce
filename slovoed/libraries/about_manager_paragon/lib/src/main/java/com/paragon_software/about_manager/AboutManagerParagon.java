package com.paragon_software.about_manager;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.extbase.SoundInfo;
import com.paragon_software.native_engine.DictionaryInfoAPI;
import com.paragon_software.native_engine.EngineInformationAPI;
import com.paragon_software.native_engine.EngineVersion;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class AboutManagerParagon extends AboutManagerAPI
{
  @NonNull
  private final DictionaryManagerAPI dictionaryManager;
  @NonNull
  private final EngineInformationAPI engineInformation;
  private Map< String, UIData > uiDataMap = new HashMap<>();

  AboutManagerParagon( @NonNull DictionaryManagerAPI dictionaryManager, @NonNull EngineInformationAPI engineInformation )
  {
    this.dictionaryManager = dictionaryManager;
    this.engineInformation = engineInformation;
  }

  @Override
  public void registerUI( @NonNull String controllerName, @NonNull Class< ? extends Activity > aClass )
  {
    UIData uiData = uiDataMap.get(controllerName);
    if ( uiData == null )
    {
      uiDataMap.put(controllerName, uiData = new UIData());
    }
    if ( uiData.activityClass == null )
    {
      uiData.activityClass = aClass;
    }
  }

  @Override
  public boolean showAbout( @NonNull Context context, @NonNull String controllerName,
                            @Nullable Dictionary.DictionaryId dictionaryId )
  {
    boolean shown = false;
    UIData uiData = uiDataMap.get(controllerName);
    if ( uiData != null )
    {
      if ( uiData.controller == null )
      {
        uiData.controller = new AboutController(this);
      }
      uiData.controller.setDictionaryId(dictionaryId);
      if ( uiData.activityClass != null )
      {
        if ( Activity.class.isAssignableFrom(uiData.activityClass) )
        {
          Intent intent = new Intent(context, uiData.activityClass);
          intent.putExtra(EXTRA_CONTROLLER_ID, controllerName);
          context.startActivity(intent);
        }
      }
      shown = true;
    }
    return shown;
  }

  @NonNull
  @Override
  AboutControllerAPI getController( @NonNull String controllerName )
  {
    UIData uiData = uiDataMap.get(controllerName);
    if ( uiData == null )
    {
      uiDataMap.put(controllerName, uiData = new UIData());
    }
    if ( uiData.activityClass == null )
    {
      uiData.controller = new AboutController(this);
    }
    return uiData.controller;
  }

  @Nullable
  @Override
  public Dictionary getDictionary( @NonNull Dictionary.DictionaryId dictionaryId )
  {
    Dictionary dictionary = null;
    if ( !dictionaryManager.isSelectAllDictionaries() )
    {
      for ( Dictionary d : dictionaryManager.getDictionaries() )
      {
        if ( d.getId().equals(dictionaryId) )
        {
          dictionary = d;
          break;
        }
      }
    }
    return dictionary;
  }

  @NonNull
  @Override
  EngineVersion getEngineVersion()
  {
    return engineInformation.getEngineVersion();
  }

  @NonNull
  @Override
  List< AboutSoundInfo > getSoundInfo(@NonNull Dictionary.DictionaryId dictionaryId )
  {
    List< AboutSoundInfo > result = new ArrayList<>();
    Dictionary dictionary = getDictionary(dictionaryId);
    if ( null != dictionary )
    {
      for ( SoundInfo soundInfo : dictionary.getSoundInfoList() )
      {
        DictionaryInfoAPI info = engineInformation.getDictionaryInfo(soundInfo.getLocation());
        if ( null != info )
        {
          result.add(new AboutSoundInfo(info.getString(DictionaryInfoAPI.Name.Product)));
        }
      }
    }
    return result;
  }

  private static class UIData
  {
    Class< ? extends Activity > activityClass;
    AboutControllerAPI          controller;
  }
}
