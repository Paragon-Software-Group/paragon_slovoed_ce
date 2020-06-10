package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.collections.MutableCollectionView;
import com.paragon_software.utils_slovoed.collections.StrictProxyCollectionView;
import com.paragon_software.utils_slovoed.collections.TransformCollectionView;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

class TransformFlashcardDataToView extends TransformCollectionView<FlashcardView,
                                                                   FlashcardBox,
                                                                   TransformFlashcardDataToView.TransformDataToView,
                                                                   BaseQuizAndFlashcardMetadata,
                                                                   TransformFlashcardDataToView.SortWrapperCv>
                                   implements AbstractFlashcardCollectionView {

  TransformFlashcardDataToView(MutableCollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> flashcards, @NonNull FlashcardMetadata.SortMode sortMode) {
    super(new SortWrapperCv(flashcards, sortMode), new TransformDataToView());
  }

  @Override
  public void checkCard(int position, boolean checked) {
    FlashcardBox flashcardBox = getCore().getItem(position);
    if ( flashcardBox.isChecked() != checked )
      getCore().update(position, FlashcardBox.setChecked(flashcardBox, checked));
  }

  @Override
  public void checkAll(boolean checked) {
    for ( int i = 0 ; i < getCore().getCount() ; i++ ) {
      FlashcardBox box = getCore().getItem(i);
      getCore().update(i, FlashcardBox.setChecked(box, checked));
    }
  }

  @Override
  public void remove( int position )
  {
    getCore().remove(position);
  }

  @Override
  public void setSortMode( @NonNull FlashcardMetadata.SortMode sortMode ) {
    getCore().set(sortMode);
    callOnMetadataChanged();
  }

  @Override
  public void activateRegularMode() {
    throw new UnsupportedOperationException();
  }

  @Override
  public void activateSelectionMode( int position ) {
    throw new UnsupportedOperationException();
  }

  @Override
  public int getCheckedCount() {
    int n = getCount(), res = 0;
    for ( int i = 0 ; i < n ; i++ ) {
      if ( getCore().getItem(i).isChecked() ) {
        res++;
      }
    }
    return res;
  }

  @Override
  public BaseQuizAndFlashcardMetadata getMetadata() {
    return new FlashcardMetadata(getCore().getSortMode(),
                                 FlashcardMetadata.Mode.Regular,
                                 new ButtonState(VisibilityState.gone, CheckState.uncheckable));
  }

  static class TransformDataToView implements TransformCollectionView.TransformFunction<FlashcardView, FlashcardBox> {
    @Override
    public FlashcardView transform( int position, FlashcardBox box ) {
      String showVariantText = box.getParent().getShowVariantText();
      showVariantText = null == showVariantText ? "<empty>" : showVariantText;
      return new FlashcardView(showVariantText, box.isChecked(), box.getPercentSuccess());
    }

    @Override
    public void onItemRangeChanged( OPERATION_TYPE type, int startPosition, int itemCount ) { }
  }

  static class SortWrapperCv extends StrictProxyCollectionView<FlashcardBox,
                                                               BaseQuizAndFlashcardMetadata,
                                                               SortCardsCv>
                             implements MutableCollectionView<FlashcardBox,
                                                              BaseQuizAndFlashcardMetadata> {
    @NonNull
    private final MutableCollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> mFlashcards;

    SortWrapperCv(@NonNull MutableCollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> flashcards, @NonNull FlashcardMetadata.SortMode sortMode) {
      super(new SortCardsCv(flashcards, sortMode));
      mFlashcards = flashcards;
    }

    void set(@NonNull FlashcardMetadata.SortMode sortMode) {
      set(new SortCardsCv(mFlashcards, sortMode));
    }

    @NonNull
    FlashcardMetadata.SortMode getSortMode() {
      return getCore().sortMode;
    }

    @Override
    public void update(int position, FlashcardBox flashcardBox) {
      getCore().update(position, flashcardBox);
    }

    @Override
    public void remove(int position) {
      getCore().remove(position);
    }

    @Override
    public void updateMetadata(@Nullable BaseQuizAndFlashcardMetadata metadata) {
      getCore().updateMetadata(metadata);
    }
  }
}
