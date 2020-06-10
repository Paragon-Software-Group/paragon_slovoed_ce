package com.paragon_software.user_core_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.google.gson.annotations.SerializedName;

import java.io.Serializable;
import java.util.Date;
import java.util.Objects;

public class ProductLicense implements Serializable
{
  // WARNING!!! Don't change @SerializedName values. They already
  // was used in persistent storage of real users devices.
  // (it has so strange names because of ProGuard)
  @SerializedName("a")
  @NonNull
  private String productId;

  @SerializedName("b")
  @Nullable
  private Date endDate;

  public ProductLicense( @NonNull String productId, @Nullable Date endDate )
  {
    this.productId = productId;
    this.endDate = endDate;
  }

  @NonNull
  public String getProductId()
  {
    return productId;
  }

  @Nullable
  public Date getEndDate()
  {
    return endDate;
  }

  @Override
  public boolean equals( Object o )
  {
    if ( this == o )
    {
      return true;
    }
    if ( o == null || getClass() != o.getClass() )
    {
      return false;
    }

    ProductLicense that = (ProductLicense) o;

    if ( !productId.equals(that.productId) )
    {
      return false;
    }
    return Objects.equals(endDate, that.endDate);
  }

  @Override
  public int hashCode()
  {
    int result = productId.hashCode();
    result = 31 * result + ( endDate != null ? endDate.hashCode() : 0 );
    return result;
  }
}
