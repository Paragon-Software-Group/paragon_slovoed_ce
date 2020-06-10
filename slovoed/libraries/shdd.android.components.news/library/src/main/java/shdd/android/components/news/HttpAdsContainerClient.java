package shdd.android.components.news;

import android.util.Pair;


public class HttpAdsContainerClient extends HttpAdsClient {
	private final Integer protocol = 2;

	protected HttpAdsContainerClient(int catalogOrProductId) {
		super(catalogOrProductId);
	}

	protected Integer getProtocol() {
		return protocol;
	}

	@Override
    protected Pair<String,String> getTargetId() {
        return new Pair<String,String>("catalog_id", String.valueOf(mCatalogOrProductId));
    }
}
