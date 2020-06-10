package com.paragon_software.sound_manager;

import io.reactivex.Single;
import io.reactivex.schedulers.Schedulers;
import okhttp3.RequestBody;
import okhttp3.ResponseBody;
import retrofit2.Retrofit;
import retrofit2.adapter.rxjava2.RxJava2CallAdapterFactory;
import retrofit2.converter.gson.GsonConverterFactory;
import retrofit2.http.Body;
import retrofit2.http.GET;
import retrofit2.http.POST;
import retrofit2.http.Path;

public interface ParagonOnlineSoundService {

  String BASE_URL = "http://oald10-staging.aks2.azure.paragonbox.com/";

  //TODO сервис пока в разработке

  static ParagonOnlineSoundService create() {

    RxJava2CallAdapterFactory rxAdapter = RxJava2CallAdapterFactory.createWithScheduler(Schedulers.io());

    Retrofit retrofit = new Retrofit.Builder()
        .baseUrl(BASE_URL)
        .addConverterFactory(GsonConverterFactory.create())
        .addCallAdapterFactory(rxAdapter)
        .build();
    return retrofit.create(ParagonOnlineSoundService.class);
  }

  @POST("/api/dictionary/file")
  Single<ResponseBody> getSoundFile(@Body RequestBody request);

  @POST("/api/dictionary/description")
  Single<ResponseBody> initSoundData(@Body RequestBody request);

  @GET("/api/dictionary/file/{id}/{token}")
  Single<ResponseBody> getSoundData(@Path("id") String id, @Path("token") String token);
}
