package shdd.android.components.news.utils;

import shdd.android.components.news.data.AdsAd;
import shdd.android.components.news.data.NewsItem;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

public class XmlParser {
    private final static SimpleDateFormat mDateFormatter = new SimpleDateFormat(
            "yyyy-MM-dd", Locale.US);
    private byte[] mAdsAndNewsRawResponse;

    public XmlParser() {
    }

    public static String getTextFromNode(Node root)	{
        StringBuilder builder = new StringBuilder();
        NodeList list = root.getChildNodes();
        for (int i = 0; i < list.getLength();i++) {
            Node node = list.item(i);
            if (node.getNodeType()==Node.TEXT_NODE || node.getNodeType() == Node.CDATA_SECTION_NODE) {
                builder.append(node.getNodeValue());
            }
        }
        return builder.toString();
    }

    public Document createDocument() {
        try {
            byte[] source = getAdsAndNewsRawResponse();
            DocumentBuilder builder;
            DocumentBuilderFactory factory = DocumentBuilderFactory
                    .newInstance();
            factory.setCoalescing(true);
            builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();

            InputStream is = new ByteArrayInputStream(source);
            return builder.parse(is);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

    }

    public static List<NewsItem> parseToNews(Document doc)
            throws ParseException {
        if (doc == null) {
            return Collections.emptyList();
        }
        NodeList newsNodes = doc.getElementsByTagName("news");

        if (newsNodes.getLength() == 0) {
            return Collections.emptyList();

        }
        Node newsNode = newsNodes.item(0);
        NodeList newsItems = newsNode.getChildNodes();
        List<NewsItem> returnList = new ArrayList<NewsItem>();
        for (int i = 0; i < newsItems.getLength(); i++) {
            Node item = newsItems.item(i);

            if (item.getNodeName().equals("item")) {

                NewsItem newsItem = new NewsItem();

                NamedNodeMap itemAttributes = item.getAttributes();
                if (itemAttributes != null) {
                    String id = itemAttributes.getNamedItem("id")
                            .getNodeValue();
                    String uTime = itemAttributes.getNamedItem("utime")
                            .getNodeValue();

                    newsItem.setId(Integer.parseInt(id));
                    newsItem.setDate(new Date(Long.parseLong(uTime) * 1000));
                }
                Element newsEl = (Element) item;
                NodeList messageList = newsEl.getElementsByTagName("message");

                for (int j = 0; j < messageList.getLength(); j++) {

                    Element message = (Element) messageList.item(j);
                    NewsItem.Message newsMessage = new NewsItem.Message();

                    NamedNodeMap messageAttributes = message.getAttributes();

                    String lang = messageAttributes.getNamedItem("lang")
                            .getNodeValue();
                    String locale = messageAttributes.getNamedItem("locale")
                            .getNodeValue();

                    String title = "";
                    NodeList titles = message.getElementsByTagName("title");
                    if (titles.getLength() == 1) {
                        title = getTextFromNode(titles.item(0));
                    }
                    String body = "";
                    NodeList bodys = message.getElementsByTagName("body");
                    if (titles.getLength() > 0) {
                        for (int bodyI = 0; bodyI < bodys.getLength(); bodyI++) {
                            Node node = bodys.item(bodyI);
                            body += getTextFromNode(node);

                        }
                    }

                    newsMessage.setTitle(title);
                    newsMessage.setLocale(locale);
                    newsMessage.setLang(lang);
                    newsMessage.setBody(body);

                    newsItem.addMessage(newsMessage);

                }

                returnList.add(newsItem);
            }
        }
        return returnList;
    }

    public static List<AdsAd> parseToAds(Document doc) throws ParseException {
        NodeList adsNodes = doc.getElementsByTagName("ads");

        if (adsNodes.getLength() == 0) {

            return Collections.emptyList();
        }
        Node adsNode = adsNodes.item(0);
        NodeList adItems = adsNode.getChildNodes();
        List<AdsAd> returnList = new ArrayList<AdsAd>();
        for (int i = 0; i < adItems.getLength(); i++) {
            Node ad = adItems.item(i);

            if (ad.getNodeName().equals("ad")) {

                AdsAd adRes = new AdsAd();

                NamedNodeMap adAttributes = ad.getAttributes();
                if (adAttributes != null) {
                    String id = adAttributes.getNamedItem("id").getNodeValue();
                    String state_date = adAttributes.getNamedItem("start_date")
                            .getNodeValue();
                    String finish_date = adAttributes.getNamedItem(
                            "finish_date").getNodeValue();
                    final String url = adAttributes.getNamedItem("url")
                            .getNodeValue();

                    adRes.setmId(Integer.parseInt(id));
                    adRes.setmStartDate(mDateFormatter.parse(state_date));
                    adRes.setmFinishDate(finishDateIncludeInPeriod(finish_date));
                    try {
                        adRes.setmAdUrl(new URL(url));
                    } catch (MalformedURLException e) {
                        e.printStackTrace();

                    }
                }
                Element adEl = (Element) ad;
                NodeList messageList = adEl.getElementsByTagName("message");

                for (int j = 0; j < messageList.getLength(); j++) {

                    Node message = messageList.item(j);

                    NamedNodeMap messageAttributes = message.getAttributes();

                    String lang = messageAttributes.getNamedItem("lang")
                            .getNodeValue();
                    String locale = messageAttributes.getNamedItem("locale")
                            .getNodeValue();
                    String url = messageAttributes.getNamedItem("url")
                            .getNodeValue();

                    try {
                        adRes.addMessage(adRes.getmId(), locale, lang, new URL(url));
                    } catch (Exception e) {
                        throw new ParseException(
                                "Not correct message url with  lang = " + lang,
                                Integer.valueOf(j));
                    }

                }

                returnList.add(adRes);
            }
        }
        return returnList;

    }

    @SuppressWarnings("deprecation")
    private static Date finishDateIncludeInPeriod(String finish_date) throws ParseException {
        Date finishDate = mDateFormatter.parse(finish_date);
        // день окончания включается в период действия акции
        finishDate.setHours(23);
        finishDate.setMinutes(59);
        finishDate.setSeconds(59);
        return finishDate;
    }


    public byte[] getAdsAndNewsRawResponse() {
        return mAdsAndNewsRawResponse;
    }

    public void setAdsAndNewsRawResponse(byte[] mAdsAndNewsRawResponse) {
        this.mAdsAndNewsRawResponse = mAdsAndNewsRawResponse;
    }
}