package com.paragon_software.dictionary_manager_ui;

import android.app.Dialog;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.FragmentManager;
import androidx.appcompat.app.AlertDialog;

public class DialogFragmentError extends DialogFragment
{
  private static final String TAG = DialogFragmentError.class.toString();

  public static void showDialog( FragmentManager fragmentManager, @NonNull String message )
  {
    if ( null != fragmentManager )
    {
      if ( fragmentManager.findFragmentByTag(TAG) == null )
      {
        DialogFragmentError errorDialog = new DialogFragmentError();
        Bundle args = new Bundle();
        args.putString("message", message);
        errorDialog.setArguments(args);
        errorDialog.show(fragmentManager, TAG);
      }
    }
  }

  @SuppressWarnings( "ConstantConditions" )
  @NonNull
  @Override
  public Dialog onCreateDialog( Bundle savedInstanceState )
  {
    String message = getArguments().getString("message");
    return new AlertDialog.Builder(getActivity()).setTitle(R.string.dictionary_manager_ui_error_dialog_title_default).setMessage(message).create();
  }
}
