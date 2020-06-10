package com.paragon_software.sound_manager;

import android.content.Context;
import android.provider.Settings;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.EngineSoundAPI;
import com.paragon_software.native_engine.data.SoundData;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.observers.DisposableSingleObserver;
import io.reactivex.schedulers.Schedulers;

import java.nio.charset.StandardCharsets;
import java.util.UUID;

public class SlovosoundOnlineSound implements OnlineSoundApi {

  public static class SlovosoundOnlineFactory implements OnlineSoundApi.Factory
  {

    private Context mContext;
    private EngineSoundAPI mEngineSound;

    @Override
    public OnlineSoundApi create() {
      return new SlovosoundOnlineSound(mContext, mEngineSound) ;
    }

    @Override
    public Factory registerContext(@Nullable Context context) {
      mContext = context;
      return this;
    }

    @Override
    public Factory registerEngineSound(@Nullable EngineSoundAPI engineSound) {
      mEngineSound = engineSound;
      return this;
    }
  }
  
  private final Context mContext;
  private SlovosoundOnlineSoundService mOnlineSoundService;
  private final static String PROTOCOL = "1";

  @NonNull
  private final EngineSoundAPI mEngineSound;


  SlovosoundOnlineSound(Context context, @NonNull EngineSoundAPI engineSound) {
    mOnlineSoundService = SlovosoundOnlineSoundService.create();
    mEngineSound = engineSound;
    mContext = context;
  }

  @Override
  public void play(Dictionary.DictionaryId dictionaryId, String soundBaseIdx, String soundKey, EngineSoundAPI.OnSoundReadyCallback callback) {
    String imei = getAndroidIdOrRandom(mContext);
    String checksum = getChecksum(PROTOCOL, soundBaseIdx.toUpperCase(), Utils.getSoundBaseVersion(dictionaryId, soundBaseIdx), soundKey, imei);
    mOnlineSoundService.getSoundData(PROTOCOL, soundBaseIdx.toUpperCase(), Utils.getSoundBaseVersion(dictionaryId, soundBaseIdx), soundKey, imei, checksum)
        .map(responseBody -> mEngineSound.getSoundDataFromSpxSynch(dictionaryId, responseBody.bytes()))
        .subscribeOn(Schedulers.computation())
        .observeOn(AndroidSchedulers.mainThread())
        .subscribe(new DisposableSingleObserver<SoundData>() {
          @Override
          public void onSuccess(SoundData soundData) {
            callback.onSoundReady(soundData);
            dispose();
          }

          @Override
          public void onError(Throwable e) {
            e.printStackTrace();
            dispose();
          }
        });
  }

  private static String getAndroidIdOrRandom(Context context) {
    try {
      final String androidId = Settings.Global.getString(context.getContentResolver(),
          Settings.Secure.ANDROID_ID);
      if (androidId != null && !"9774d56d682e549c".equals(androidId)) {// bug 2.2
        return UUID.nameUUIDFromBytes(androidId.getBytes(StandardCharsets.UTF_8)).toString();
      } else {
        return UUID.randomUUID().toString();
      }
    } catch (Exception e) {
      return UUID.randomUUID().toString();
    }
  }

  private String getChecksum(String protocol, String soundBaseIdx, String soundBaseVersion, String soundKey, String imei) {
    StringBuilder sb = new StringBuilder();
    sb.append(protocol).append("::")
        .append(soundBaseIdx).append("::")
        .append(soundBaseVersion).append("::")
        .append(soundKey).append("::").append(imei);
    return Utils.getSig(sb.toString());
  }
}
