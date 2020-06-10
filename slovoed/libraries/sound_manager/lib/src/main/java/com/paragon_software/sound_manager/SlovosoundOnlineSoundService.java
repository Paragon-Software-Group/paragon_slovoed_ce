package com.paragon_software.sound_manager;

import io.reactivex.Single;
import io.reactivex.schedulers.Schedulers;
import okhttp3.ResponseBody;
import retrofit2.Retrofit;
import retrofit2.adapter.rxjava2.RxJava2CallAdapterFactory;
import retrofit2.converter.gson.GsonConverterFactory;
import retrofit2.http.GET;
import retrofit2.http.Query;

public interface SlovosoundOnlineSoundService {

  String BASE_URL = "https://slovosound.penreader.com/v2/slovosound/";

  static SlovosoundOnlineSoundService create() {

    RxJava2CallAdapterFactory rxAdapter = RxJava2CallAdapterFactory.createWithScheduler(Schedulers.io());

    Retrofit retrofit = new Retrofit.Builder()
        .baseUrl(BASE_URL)
        .addConverterFactory(GsonConverterFactory.create())
        .addCallAdapterFactory(rxAdapter)
        .build();
    return retrofit.create(SlovosoundOnlineSoundService.class);
  }

  @GET(".")
  Single<ResponseBody> getSoundData(@Query("protocol") String protocol,
                                    @Query("base_id") String base_id,
                                    @Query("base_version") String base_version,
                                    @Query("sound_text") String sound_text,
                                    @Query("imei") String imei,
                                    @Query("checksum") String checksum);
}
