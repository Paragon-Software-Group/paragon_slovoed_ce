package com.paragon_software.utils_slovoed.pdahpc;

import com.google.gson.annotations.SerializedName;

public final class Extra {
    @SerializedName("acesProperties")
    private AcesProperties acesProperties;
    @SerializedName("eacProperties")
    private EacProperties eacProperties;

    public AcesProperties getAcesProperties() { return acesProperties; }

    public EacProperties getEacProperties() { return eacProperties; }

}
