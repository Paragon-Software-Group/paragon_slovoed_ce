package com.paragon_software.sound_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.EngineSoundAPI;
import com.paragon_software.native_engine.data.SoundData;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.observers.DisposableSingleObserver;
import io.reactivex.schedulers.Schedulers;
import okhttp3.RequestBody;
import org.json.JSONException;
import org.json.JSONObject;


public class ParagonOnlineSound implements OnlineSoundApi {

  public static class ParagonOnlineFactory implements OnlineSoundApi.Factory
  {

    private Context mContext;
    private EngineSoundAPI mEngineSound;

    @Override
    public OnlineSoundApi create() {
      return new ParagonOnlineSound(mEngineSound) ;
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

  private ParagonOnlineSoundService mOnlineSoundService;

  @NonNull
  private final EngineSoundAPI mEngineSound;
  private static final String BASEURL = "/api/dictionary/file";
  //TODO скрыть строку с ключем
  private final static String RSAKEY = "qwerty";


  ParagonOnlineSound(@NonNull EngineSoundAPI engineSound) {
    mOnlineSoundService = ParagonOnlineSoundService.create();
    mEngineSound = engineSound;
  }

  @Override
  public void play(Dictionary.DictionaryId dictionaryId, String soundBaseIdx, String soundKey, EngineSoundAPI.OnSoundReadyCallback callback)
  {
    String token = getToken(dictionaryId, soundBaseIdx, soundKey);
    mOnlineSoundService.initSoundData(createRequestBody(dictionaryId, soundBaseIdx, soundKey))
        .map(responseBody ->mOnlineSoundService.getSoundData(token, getResultToken(token)))
        .map(responseBody -> mEngineSound.getSoundDataFromSpxSynch(dictionaryId, responseBody.blockingGet().bytes()))
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

  private RequestBody createRequestBody(Dictionary.DictionaryId dictionaryId, String soundBaseIdx, String soundKey)
  {
    return RequestBody.create(okhttp3.MediaType.parse("application/json; charset=utf-8"),getJsonRequest(dictionaryId, soundBaseIdx, soundKey));
  }

  private String getJsonRequest(Dictionary.DictionaryId dictionaryId, String soundBaseIdx, String soundKey)
  {

    JSONObject json = new JSONObject();
    try {
      json.put("filePath", soundKey);
      json.put("sku", soundBaseIdx);
      json.put("token", getToken(dictionaryId, soundBaseIdx, soundKey));
      json.put("version", Utils.getSoundBaseVersion(dictionaryId, soundBaseIdx).substring(2));
    } catch (JSONException e) {
      e.printStackTrace();
    }
    return json.toString();
  }

  private String getToken(Dictionary.DictionaryId dictionaryId, String soundBaseIdx, String soundKey) {
    return Utils.getSig(BASEURL + soundBaseIdx +  Utils.getSoundBaseVersion(dictionaryId, soundBaseIdx).substring(2) + soundKey + RSAKEY);
  }

  private String getResultToken(String id) {
    return Utils.getSig(BASEURL + "/" + id + "/" + RSAKEY);
  }
}
