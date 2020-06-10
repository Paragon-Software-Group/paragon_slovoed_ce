package com.paragon_software.word_of_day;

import java.io.Serializable;

import com.google.gson.annotations.SerializedName;

public class WotDItem implements Serializable {

	// WARNING!!! Don't change @SerializedName values. They already
	// was used in persistent storage of real users devices.
	// (it has so strange names because of ProGuard)
	@SerializedName("a")
	private final long date;
	@SerializedName("b")
	private final String headword;
	@SerializedName("c")
	private final String partOfSpeech;
	@SerializedName("d")
	private final String entryId;
	@SerializedName("e")
	private final String cefrLevel;
	@SerializedName("f")
	private final WotDLinkType linkType;
	@SerializedName("g")
	private final String linkLabel;
	@SerializedName("h")
	private final String linkURL;
	@SerializedName("i")
	private final boolean isViewed;

	public WotDItem(Long date, String headword,
					String partOfSpeech, String entryId, String cefrLevel,
					WotDLinkType linkType, String linkLabel,
					String linkURL, boolean isViewed)
	{
		this.date = date;
		this.headword = headword;
		this.partOfSpeech = partOfSpeech;
		this.entryId = entryId;
		this.cefrLevel = cefrLevel;
		this.linkType = linkType;
		this.linkLabel = linkLabel;
		this.linkURL = linkURL;
		this.isViewed = isViewed;
	}

	public Long getDate() {
		return date;
	}

	public String getHeadword() {
		return headword;
	}

	public String getPartOfSpeech() {
		return partOfSpeech;
	}

	public String getEntryId() {
		return entryId;
	}

	public String getCefrLevel() {
		return cefrLevel;
	}

	public WotDLinkType getLinkType() {
		return linkType;
	}

	public String getLinkLabel() {
		return linkLabel;
	}

	public String getLinkURL() {
		return linkURL;
	}

	public boolean getIsViewed() {
		return isViewed;
	}

    @Override
    public boolean equals(Object o)
    {
        if (this == o)
        {
            return true;
        }
        if ( !(o instanceof WotDItem) )
        {
            return false;
        }
        WotDItem item = (WotDItem) o;
        return toString().equals(item.toString());
    }

    @Override
    public int hashCode()
    {
        return toString().hashCode();
    }

    public boolean equalsWithoutViewed(WotDItem item)
	{
	    if( item == null )
	    {
            return false;
        }
		if( this.equals(item) )
		{
			return true;
		}
		else
		{
			return date == item.date &&
					headword.equals(item.headword) &&
					partOfSpeech.equals(item.partOfSpeech) &&
					entryId.equals(item.entryId) &&
					cefrLevel.equals(item.cefrLevel) &&
					linkType == item.linkType &&
					linkLabel.equals(item.linkLabel) &&
					linkURL.equals(item.linkURL);
		}
	}

    @Override
	public String toString()
	{
		return "WotDItem{" + "date=" + date + ", headword='" + headword + '\'' + ", partOfSpeech='" + partOfSpeech
				+ '\'' + ", entryId='" + entryId + '\'' + ", cefrLevel='" + cefrLevel + '\'' + ", linkType=" + linkType
				+ ", linkLabel='" + linkLabel + '\'' + ", linkURL='" + linkURL + '\'' + ", isViewed='" + isViewed + '\'' + '}';
	}
}
