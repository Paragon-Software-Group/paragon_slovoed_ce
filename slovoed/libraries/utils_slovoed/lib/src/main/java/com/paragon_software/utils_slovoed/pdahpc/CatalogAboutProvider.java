package com.paragon_software.utils_slovoed.pdahpc;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.text.LocalizedString;

import static com.paragon_software.utils_slovoed.pdahpc.LocalizeStringHelper.getLocalizedString;

public class CatalogAboutProvider
{
    private final CatalogAbout catalogAbout;
    private final String defaultLanguage;

    public CatalogAboutProvider( @NonNull CatalogAbout catalogAbout,
                                 @NonNull String defaultLanguage )
    {
        this.catalogAbout = catalogAbout;
        this.defaultLanguage = defaultLanguage;
    }

    public LocalizedString getCopyright() {
        return getLocalizedString(catalogAbout.getCopyrightMap(), defaultLanguage);
    }

    public LocalizedString getName() {
        return getLocalizedString(catalogAbout.getNameMap(), defaultLanguage);
    }

    public LocalizedString getWeb() {
        return getLocalizedString(catalogAbout.getWebMap(), defaultLanguage);
    }

    public LocalizedString getFaq() {
        return getLocalizedString(catalogAbout.getFaqMap(), defaultLanguage);
    }

    public LocalizedString getSupportEmail() {
        return getLocalizedString(catalogAbout.getSupportEmailMap(), defaultLanguage);
    }

    public LocalizedString getProvidedBrand() {
        return getLocalizedString(catalogAbout.getProvidedMap(), defaultLanguage);
    }
}
