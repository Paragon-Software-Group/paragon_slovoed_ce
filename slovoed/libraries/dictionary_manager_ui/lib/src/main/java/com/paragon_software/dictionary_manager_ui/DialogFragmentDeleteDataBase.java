package com.paragon_software.dictionary_manager_ui;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Parcelable;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.FragmentManager;
import androidx.appcompat.app.AlertDialog;

import com.paragon_software.dictionary_manager.DictionaryControllerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.DictionaryManagerHolder;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;

import static com.paragon_software.dictionary_manager_ui.MyDictionariesFragment.MY_DICTIONARIES_CONTROLLER_TAG;

public class DialogFragmentDeleteDataBase extends DialogFragment {

	public static final String ARG_COMPONENT = "com.paragon_software.dictionary_manager_ui.ARG_COMPONENT";
	@NonNull
	@Override
	public Dialog onCreateDialog(final Bundle savedInstanceState) {

		return new AlertDialog.Builder(getContext())
				.setMessage(R.string.dictionary_manager_ui_delete_dictionary_database)
				.setNegativeButton(R.string.utils_slovoed_ui_common_cancel, null)
				.setPositiveButton(R.string.dictionary_manager_ui_ok, new DialogInterface.OnClickListener() {
					@Override
					public void onClick(DialogInterface dialogInterface, int i) {
						DictionaryManagerAPI dictionaryManagerAPI = DictionaryManagerHolder.getManager();
						DictionaryControllerAPI controller = dictionaryManagerAPI.createController(MY_DICTIONARIES_CONTROLLER_TAG);
						Bundle args = getArguments();
						DictionaryComponent component = null;
						if(args != null) {
							Parcelable p = args.getParcelable(ARG_COMPONENT);
							if(p instanceof DictionaryComponent)
								component = (DictionaryComponent) p;
						}
						if (controller != null && component != null) {
								controller.remove(component);
						}
					}
				})
				.create();
	}

	public static void show( @Nullable FragmentManager manager, @NonNull DictionaryComponent component )
	{
		if ( null != manager )
		{
			DialogFragmentDeleteDataBase dialog = new DialogFragmentDeleteDataBase();
			Bundle args = new Bundle();
			args.putParcelable(DialogFragmentDeleteDataBase.ARG_COMPONENT, component);
			dialog.setArguments(args);
			dialog.show(manager, DialogFragmentDeleteDataBase.class.toString());
		}
	}
}
