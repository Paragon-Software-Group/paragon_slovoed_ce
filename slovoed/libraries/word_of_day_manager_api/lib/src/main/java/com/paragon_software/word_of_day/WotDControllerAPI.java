package com.paragon_software.word_of_day;

import android.content.Context;
import android.content.Intent;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.FragmentManager;
import android.util.Pair;
import android.view.View;

import java.util.List;

import com.paragon_software.hint_manager.HintParams;
import com.paragon_software.hint_manager.HintType;
import io.reactivex.Completable;
import io.reactivex.Observable;
import io.reactivex.Single;

public abstract class WotDControllerAPI
{
  abstract Completable update();
  abstract void clickItem( @NonNull WotDItem item );

  abstract Observable<WotDItem> getCurrentItem();

  abstract void openArticle(Context context, String entryId);

  public abstract Observable<ErrorType> getErrorMessage();
  public abstract Observable<Float> getListFontSize();

  public abstract Observable<Boolean> getSoundState();

  abstract Observable<List<WotDItem>> getWords();
  abstract Observable<Boolean> getUpdatingState();

  public abstract boolean isReceiveWotdSettingEnabled();

  abstract WotDNotificationOptions getNotificationOptions();
  abstract String getString( WotDRes wotDRes  );

  public abstract void setPlaySoundChannel(@NonNull String soundChannel);
  public abstract void playBritishSound(String entryId);
  public abstract void playAmericanSound(String entryId);

  public abstract Single<Intent> getShareWotDItemCardIntent(@NonNull View view,
                                                            String cardDate,
                                                            String chooserText,
                                                            String extraText,
                                                            WotDCardTakenListener listener );

  public abstract Completable deleteShareWotDItemImages( @NonNull Context context );

  abstract boolean showHintManagerDialog( @NonNull HintType hintType, @Nullable FragmentManager fragmentManager, @Nullable HintParams hintParams );
  abstract Observable< Pair<HintType, HintParams> > getShowHintObservable();

  abstract boolean isPreviewMode();
}
