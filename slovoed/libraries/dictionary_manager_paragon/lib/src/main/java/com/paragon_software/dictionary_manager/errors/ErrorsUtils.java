package com.paragon_software.dictionary_manager.errors;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.List;

import com.paragon_software.license_manager_api.LicenseManager;
import com.paragon_software.license_manager_api.exceptions.*;

public class ErrorsUtils
{

  public static OperationType from( LicenseManager.OPERATION_TYPE lmOperation )
  {
    switch ( lmOperation )
    {
      case UPDATE_FEATURES:
        return OperationType.UPDATE_PURCHASES;
      case CONSUME_FEATURE:
        return OperationType.CONSUME;
      case ACTIVATE_FEATURE:
        return OperationType.UNDEFINED;
    }
    return OperationType.UNDEFINED;
  }

  @NonNull
  public static List< ErrorType > fromException( @NonNull ActivateException[] exceptions )
  {
    List< ErrorType > errors = new ArrayList<>(exceptions.length);
    for ( ActivateException exception : exceptions )
    {
      errors.add(fromException(exception));
    }
    return errors;
  }

  @NonNull
  public static ErrorType fromException( @NonNull ActivateException e )
  {
    if ( e instanceof ItemNotOwnedException )
    {
      return ErrorType.PURCHASE_ITEM_NOT_OWNED;
    }
    else if ( e instanceof ItemAlreadyOwnedException )
    {
      return ErrorType.PURCHASE_ITEM_ALREADY_OWNED;
    }
    else if ( e instanceof ItemUnavailableException )
    {
      return ErrorType.PURCHASE_ITEM_UNAVAILABLE;
    }
    else if ( e instanceof UserCancelException )
    {
      return ErrorType.PURCHASE_USER_CANCEL;
    }
    else if ( e instanceof PurchaseExceptions )
    {
      return ErrorType.PURCHASE_COMMON_ERROR;
    }
    else if ( e instanceof InvalidUserSource )
    {
      return ErrorType.ACCOUNT_INVALID_USER_SOURCE;
    }
    else if ( e instanceof UserAccountManagementException )
    {
      return ErrorType.ACCOUNT_MANAGEMENT_EXCEPTION;
    }
    else if ( e instanceof InterruptException )
    {
      return ErrorType.INTERRUPT_EXCEPTION;
    }
    else if ( e instanceof InvalidActivationSourceException )
    {
      return ErrorType.INVALID_ACTIVATION_SOURCE;
    }
    else if ( e instanceof BillingUnavailableException )
    {
      return ErrorType.BILLING_UNAVAILABLE;
    }
    return ErrorType.UNDEFINED;
  }
}
