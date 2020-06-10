package com.paragon_software.agreement_screen_manager;

import android.os.Bundle;
import androidx.annotation.NonNull;
import android.text.Html;

import com.paragon_software.agreement_screen_manager_ui.R;


public abstract class AgreementBaseEULAActivity extends AgreementBaseActivity
{
  @Override
  protected void onCreate( Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
  }

  @NonNull
  @Override
  protected String getAgreeInstructionText()
  {
    return getString(R.string.agreement_screen_manager_ui_agree_instruction_eula_text);
  }

  @NonNull
  @Override
  protected CharSequence getMainText()
  {
    return Html.fromHtml(getString(R.string.agreement_screen_manager_ui_eula_text));
  }

  @NonNull
  @Override
  protected CharSequence getOnlineInvitationText()
  {
    return Html.fromHtml(getString(R.string.agreement_screen_manager_ui_online_eula_invitation));
  }

  @Override
  protected String getLastUpdatedDate()
  {
    return getString(R.string.agreement_screen_manager_ui_last_updated_eula);
  }
}
