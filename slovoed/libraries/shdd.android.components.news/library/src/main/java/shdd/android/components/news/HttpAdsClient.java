package shdd.android.components.news;

import android.net.Uri;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.Pair;

import shdd.android.components.news.data.AdsAd;
import shdd.android.components.news.data.NewsItem;
import shdd.android.components.news.registration.FirebaseCm;
import shdd.android.components.news.registration.ParagonGcmRegisterer;
import shdd.android.components.news.storage.BannerStorage.BannerSize;
import shdd.android.components.news.utils.ShddLog;
import shdd.android.components.news.utils.XmlParser;

import org.w3c.dom.Document;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URL;
import java.security.InvalidParameterException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Locale;

public class HttpAdsClient {
    public static final int READ_TIMEOUT = 5000;
    public static final int CONNECT_TIMEOUT = 10000;
    private static final int TEN_SECONDS = 10000;
    public final Integer mCatalogOrProductId;
    protected final XmlParser xmlParser = new XmlParser();
    private final Integer protocol = 1;
    protected boolean isUpdate = false;
    private List<AdsAd> mAdsList = Collections.emptyList();
    private List<NewsItem> mNewsList = Collections.emptyList();
    private Long mLastResponseTime;

    protected HttpAdsClient(int catalogOrProductId) {
        mCatalogOrProductId = catalogOrProductId;
    }

    public static HttpAdsClient getAdsClient() {
        switch (Resources.getters.getNewsMode()) {
            case CONTAINER_MODE:
                return new HttpAdsContainerClient(
                        Integer.parseInt(Resources.getters.getString(Resources.Strings.CATALOG_OR_PRODUCT_ID)));
            case STANDALONE_MODE:
                return new HttpAdsClient(
                        Integer.parseInt(Resources.getters.getString(Resources.Strings.CATALOG_OR_PRODUCT_ID)));
            default:
                throw new InvalidParameterException("Undefined HttpAdsClient for mode:" + Resources.getters.getNewsMode());
        }

    }

    public static String getAdsUri() {
        return Resources.ADS_URI;
    }

    private Params updateDeviceParams(Params params) {
        params.setImei();
        params.setPushId(getRegistrationId());
        params.setDevel(Resources.getters.isInTestMode());
        params.setPrcs(Resources.getters.getPrcs());
        params.setModification(Resources.getters
                .getString(Resources.Strings.HARDCODEDCONSTANTS_APPSTORE_BUILD_ADSMODIFICATIONNAME));
        return params;
    }

    @Nullable
    private String getRegistrationId() {
        String paragonRegId = ParagonGcmRegisterer.getRegId();
        String googleRegId = FirebaseCm.getTokenSync();
        boolean fullyRegistered = null != googleRegId && googleRegId.equals(paragonRegId);
        return fullyRegistered ? paragonRegId : null;
    }

    protected Params getParams() {
        Params p = updateDeviceParams(new Params());
        String locale = Resources.getters.getString(Resources.Strings.LOCALE);
        if (!TextUtils.isEmpty(locale))
            p.setLocale(locale);
        return p;
    }

    private boolean sendPost(URI uri, String registration_id, boolean devel) {
        HttpURLConnection conn = null;
        try {

            URL url = new URL(uri.toString());
            conn = (HttpURLConnection) url.openConnection();
            conn.setReadTimeout(READ_TIMEOUT);
            conn.setConnectTimeout(CONNECT_TIMEOUT);
            conn.setRequestMethod("POST");

            String prcs = Resources.getters.getPrcs();
            String pkey = Resources.getters.getPkey();
            Uri.Builder builder = new Uri.Builder();
            builder.appendQueryParameter("gcm", "1");

            Pair<String, String> targetIdPair = getTargetId();
            builder.appendQueryParameter(targetIdPair.first, targetIdPair.second);

            if (!TextUtils.isEmpty(registration_id))
                builder.appendQueryParameter("registration_id", registration_id);


            if (!TextUtils.isEmpty(prcs))
                builder.appendQueryParameter("prcs", prcs);

            if (!TextUtils.isEmpty(pkey))
                builder.appendQueryParameter("pkey", pkey);

            if (devel)
                builder.appendQueryParameter("devel", "1");

            String query = builder.build().getEncodedQuery();

            conn.setDoInput(true);
            conn.setDoOutput(true);
            conn.setRequestProperty("Host", uri.getAuthority());
            conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
            conn.setUseCaches(false);
            OutputStream os = conn.getOutputStream();

            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(os, "UTF-8"));
            writer.write(query);
            writer.flush();
            writer.close();
            os.close();
            conn.connect();
            boolean res = conn.getResponseCode() == HttpURLConnection.HTTP_OK;
            ShddLog.d("shdd", "sendPost : " + uri.toString() + "/" + query);
            ShddLog.d("shdd", "sendPost response OK : " + res);
            return res;

        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            if (null != conn)
                conn.disconnect();
        }
    }

    protected Pair<String, String> getTargetId() {
        return new Pair<String, String>("product_id", String.valueOf(mCatalogOrProductId));
    }

    /**
     * Unregister on paragon push server
     *
     * @param uri
     * @param registration_id
     * @return true when HttpRequest result = 200 OK
     */
    public boolean unregisterToPush(URI uri, String registration_id) {
        return sendPost(URI.create(uri.toString() + "/sign_out"),
                registration_id, false);
    }

    /**
     * Register on paragon push server
     *
     * @param uri
     * @param registration_id
     * @return true when HttpRequest result = 200 OK
     */
    public boolean registerToPush(URI uri, String registration_id) {
        return sendPost(URI.create(uri.toString() + "/sign"), registration_id,
                Resources.getters.isInTestMode());
    }

    public boolean pullAdsAndNews(URI uri, Params params) {
        HttpURLConnection conn = null;
        try {
            if (hasCachedResponse())
                return true;

            Uri.Builder builder = new Uri.Builder();
            Pair<String, String> productOrCatalogParamPair = getTargetId();
            builder.appendQueryParameter(productOrCatalogParamPair.first, productOrCatalogParamPair.second);
            builder.appendQueryParameter("protocol", getProtocol().toString());
            builder.appendQueryParameter("imei", params.imei);

            if (params.locale != null)
                builder.appendQueryParameter("locale", params.locale);
            if (params.from != null)
                builder.appendQueryParameter("from", params.from);
            if (params.push != null)
                builder.appendQueryParameter("push", params.push);
            if (params.account != null)
                builder.appendQueryParameter("account", params.account);
            if (params.push_id != null)
                builder.appendQueryParameter("push_id", params.push_id);
            if (params.modification != null)
                builder.appendQueryParameter("modification", params.modification);
            if (params.prcs != null)
                builder.appendQueryParameter("prcs", params.prcs);
            if (params.devel != null)
                builder.appendQueryParameter("devel", params.devel);

            URL url = new URL(uri.toString() + "?" + builder.build().getEncodedQuery());

            conn = (HttpURLConnection) url.openConnection();
            conn.setReadTimeout(READ_TIMEOUT);
            conn.setConnectTimeout(CONNECT_TIMEOUT);
            conn.setRequestMethod("GET");

            ShddLog.d("shdd", "Http GET; Url: " + url);
            int responseCode = conn.getResponseCode();
            InputStream is = conn.getInputStream();
            if (responseCode == HttpURLConnection.HTTP_OK) {
                mLastResponseTime = new Date().getTime();
                xmlParser.setAdsAndNewsRawResponse(Utils.inputStreamToByteArray(is));
                is.close();
                isUpdate = true;
                return true;
            } else if (responseCode == HttpURLConnection.HTTP_BAD_REQUEST) {
                String content = Utils.inputStreamToString(is);
                is.close();
                throw new IllegalArgumentException("Not correct request" + content);
            } else if (responseCode == HttpURLConnection.HTTP_INTERNAL_ERROR) {
                return false;
            }
            return false;
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (null != conn)
                conn.disconnect();
        }
        return false;
    }

    private boolean hasCachedResponse() {
        return xmlParser.getAdsAndNewsRawResponse() != null &&
                mLastResponseTime != null && new Date().getTime() - mLastResponseTime < TEN_SECONDS;
    }

    protected boolean parseLastResponse() {
        if (isUpdate) {
            if (xmlParser.getAdsAndNewsRawResponse() == null) {
                return false;
            }

            try {
                Document doc = xmlParser.createDocument();
                if (doc != null) {
                    mAdsList = XmlParser.parseToAds(doc);
                    mNewsList = XmlParser.parseToNews(doc);
                    isUpdate = false;
                    return true;
                } else {
                    return false;
                }
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        } else {
            return false;
        }
    }

    /**
     * Parse xml from server with ads
     *
     * @throws ParseException
     */
    public List<AdsAd> getAds() throws ParseException {
        parseLastResponse();
        return mAdsList;
    }

    /**
     * Parse xml from server with news
     *
     * @throws ParseException
     */
    public List<NewsItem> getNews() throws ParseException {
        parseLastResponse();
        return mNewsList;
    }

    public byte[] getBannerPNG(URL baseUrl, int width, int height) throws IOException {
        if (width < 0 || height < 0)
            throw new IllegalArgumentException("width and height must be > 0");

        HttpURLConnection conn = null;
        try {
            Uri.Builder builder = new Uri.Builder();
            if (width != 0 && height != 0) {
                builder.appendQueryParameter("size", String.valueOf(width) + "x" + String.valueOf(height));
            }

            URL url = new URL(baseUrl.toString() + "?" + builder.build().getEncodedQuery());
            conn = (HttpURLConnection) url.openConnection();
            conn.setReadTimeout(READ_TIMEOUT);
            conn.setConnectTimeout(CONNECT_TIMEOUT);
            conn.setRequestMethod("GET");

            ShddLog.d("shdd", "Http GET; Url: " + url);
            if (conn.getResponseCode() == HttpURLConnection.HTTP_OK) {
                return Utils.inputStreamToByteArray(conn.getInputStream());
            }
            return null;
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (null != conn)
                conn.disconnect();
        }
        return null;
    }

    protected Integer getProtocol() {
        return protocol;
    }

    /**
     * @param ad - object to fill
     * @return true if succes
     */
    public boolean loadImageBytes(AdsAd ad, BannerSize bannerSize) {
        try {
            byte[] bytes = getBannerPNG(ad.getMessageByLocale(null).getURL(), bannerSize.width, bannerSize.height);
            if (null == bytes || 0 == bytes.length)
                throw new RuntimeException("No image bytes readed from stream");
            ad.getMessageByLocale(null).setBytesImg(bytes);
            ad.getMessageByLocale(null).setResolutionImg(bannerSize.toString());
        } catch (Exception e) {
            ShddLog.e("shdd", "Cant load image bytes. Url:" + ad.getMessageByLocale(null).getURL().toString()
                    + " size:" + bannerSize.toString());
            return false;
        }
        return true;
    }

    public static class Params {

        private String from;
        private String push;
        private String account;
        private String devel;
        private String imei;
        private String push_id;
        private String modification;
        private String locale;
        private String prcs;

        public void setFrom(Date from) {
            this.from = new SimpleDateFormat("yyyy-MM-dd", Locale.US).format(from);
        }

        public void setFromPush(boolean push) {
            this.push = push ? "1" : null;
        }

        public void setAccount(String account) {
            this.account = account;
        }

        public void setDevel(boolean devel) {
            this.devel = devel ? "1" : null;
        }

        public void setImei() {
            this.imei = Resources.getters
                    .getDeviceId();
        }

        public void setPushId(String pushId) {
            this.push_id = pushId;
        }

        public void setModification(String modification) {
            this.modification = modification;
        }

        public void setLocale(String locale) {
            this.locale = locale;
        }

        public void setPrcs(String prcs) {
            this.prcs = prcs;
        }

    }

    private static class Utils {

        private static String inputStreamToString(InputStream is) {
            String line;
            StringBuilder sb = new StringBuilder();
            BufferedReader br = null;
            try {
                br = new BufferedReader(new InputStreamReader(is));
                while ((line = br.readLine()) != null) {
                    sb.append(line);
                }
                return sb.toString();
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                try {
                    if (null != br)
                        br.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            return null;
        }

        private static byte[] inputStreamToByteArray(InputStream is) {
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            int nRead;
            byte[] data = new byte[1024];
            byte[] bytes = new byte[0];
            try {
                while ((nRead = is.read(data, 0, data.length)) != -1) {
                    buffer.write(data, 0, nRead);
                }

                buffer.flush();
                bytes = buffer.toByteArray();
                buffer.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            return bytes;
        }
    }
}
