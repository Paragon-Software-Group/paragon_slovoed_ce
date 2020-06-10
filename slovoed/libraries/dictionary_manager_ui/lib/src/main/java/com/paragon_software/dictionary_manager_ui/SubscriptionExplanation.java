package com.paragon_software.dictionary_manager_ui;

import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.FragmentManager;
import androidx.appcompat.app.AlertDialog;

import com.paragon_software.dictionary_manager.DictionaryPrice;
import com.paragon_software.dictionary_manager.DictionaryPrice.TypePeriodSubscription;
import com.paragon_software.dictionary_manager.SubscriptionPurchase;

public class SubscriptionExplanation extends DialogFragment
{
  public static final  String ARG_SUBS_PURCHASE = "arg_subs_purchase";
  public static final  String ARG_PACKAGE_NAME  = "arg_package_name";
  private static final String TAG               = SubscriptionExplanation.class.toString();
  private UserAction userActionCallback;

  public static void showDialog( FragmentManager fragmentManager, SubscriptionPurchase purchase, String packageName )
  {
    if ( null != fragmentManager )
    {
      if ( fragmentManager.findFragmentByTag(TAG) == null )
      {
        SubscriptionExplanation dialog = new SubscriptionExplanation();
        Bundle args = new Bundle();
        args.putParcelable(ARG_SUBS_PURCHASE, purchase);
        args.putString(ARG_PACKAGE_NAME, packageName);
        dialog.setArguments(args);
        dialog.show(fragmentManager, TAG);
      }
    }
  }

  @Override
  public void onAttach( Context context )
  {
    super.onAttach(context);
    try
    {
      userActionCallback = UserAction.class.cast(context);
    }
    catch ( Exception ignore )
    {
      userActionCallback = null;
    }
  }

  @SuppressWarnings( "ConstantConditions" )
  @NonNull
  @Override
  public Dialog onCreateDialog( Bundle savedInstanceState )
  {
    Bundle args = getArguments();
    final SubscriptionPurchase purchase = args.getParcelable(ARG_SUBS_PURCHASE);
    final String packageName = args.getString(ARG_PACKAGE_NAME);

    DialogInterface.OnClickListener neutralListener = new DialogInterface.OnClickListener()
    {
      @Override
      public void onClick( DialogInterface dialogInterface, int i )
      {
        if (null != userActionCallback)
        {
          userActionCallback.onUserGoDeepLinkForManagingSubscription(purchase, packageName);
        }
        dialogInterface.dismiss();
      }
    };
    DialogInterface.OnClickListener okListener = new DialogInterface.OnClickListener()
    {
      @Override
      public void onClick( DialogInterface dialogInterface, int i )
      {
        dialogInterface.dismiss();
      }
    };
    return new AlertDialog.Builder(getActivity()).setMessage(getMessage(purchase)).setNeutralButton(R.string.dictionary_manager_ui_manage_subs_in_gp, neutralListener).setPositiveButton(R.string.dictionary_manager_ui_ok, okListener).create();
  }

  private String getMessage( SubscriptionPurchase purchase )
  {
    return getString(R.string.dictionary_manager_ui_subscription_explanation_dialog_msg,
                     getSubscriptionPeriodAndPrice(purchase), getString(R.string.dictionary_manager_ui_manage_subs_in_gp));
  }

  @NonNull
  private String getSubscriptionPeriodAndPrice( SubscriptionPurchase purchase )
  {
    String priceAndCurrency =
        Utils.convertPriceToString(purchase.getPriceValue()) + purchase.getPriceCurrency().getSymbol();
    DictionaryPrice.PeriodSubscription period = purchase.getPeriod();

    int stringResId;
    if ( period.type.equals(TypePeriodSubscription.YEAR) )
    {
      stringResId = R.plurals.dictionary_manager_ui_active_subscription_year_with_price;
    }
    else if ( period.type.equals(TypePeriodSubscription.MONTH) )
    {
      stringResId = R.plurals.dictionary_manager_ui_active_subscription_month_with_price;
    }
    else if ( period.type.equals(TypePeriodSubscription.WEEK) )
    {
      stringResId = R.plurals.dictionary_manager_ui_active_subscription_week_with_price;
    }
    else
    {
      return priceAndCurrency;
    }
    return getResources().getQuantityString(stringResId, period.quantity, period.quantity, priceAndCurrency);
  }

  public interface UserAction
  {
    void onUserGoDeepLinkForManagingSubscription( @NonNull SubscriptionPurchase purchase, @NonNull String packageName );
  }
}
