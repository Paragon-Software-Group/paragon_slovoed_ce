package com.paragon_software.search_manager;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.native_engine.SearchEngineAPI;
import com.paragon_software.utils_slovoed.directory.Directory;
import com.paragon_software.utils_slovoed.directory.MockDirectory;

import androidx.annotation.NonNull;
import io.reactivex.Single;
import io.reactivex.subjects.SingleSubject;

public abstract class MockSearchEngine implements SearchEngineAPI {
  @NonNull
  private
  SingleSubject<Directory<ArticleItem>> resultCallback = SingleSubject.create();
  private MockDirectory rootDirectory;

  public MockSearchEngine(MockDirectory dir) {
    rootDirectory = dir;
  }

  @Override
  public Single<Directory<ArticleItem>> getPreloadedFavorites(@NonNull Dictionary.DictionaryId dictionaryId) {
    resultCallback.onSuccess(rootDirectory);
    return resultCallback;
  }
}
