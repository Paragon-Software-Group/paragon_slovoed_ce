package com.paragon_software.article_manager.dialog;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.FragmentActivity;
import androidx.appcompat.app.AlertDialog;

import com.paragon_software.common_ui.R;

public class DictionariesMissingPurchasesDialogFragment extends DialogFragment
{
  @NonNull
  @Override
  public Dialog onCreateDialog( Bundle savedInstanceState )
  {
    setCancelable(false);
    DialogInterface.OnClickListener onClickListener = (dialogInterface, i) -> {
      FragmentActivity activity = getActivity();
      if (activity instanceof Actions)
      {
        ( (Actions) activity ).onOkUIMissingPurchases();
      }
    };
    //noinspection ConstantConditions
    AlertDialog.Builder builder =
        new AlertDialog.Builder(getContext()).setMessage(R.string.article_manager_ui_share_error_missing_purchases_msg)
                                             .setPositiveButton(android.R.string.ok, onClickListener);
    return builder.create();
  }

  public interface Actions
  {
    void onOkUIMissingPurchases();
  }
}
