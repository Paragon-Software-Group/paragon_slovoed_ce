package shdd.android.components.news.data;

import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

public class NewsItem {
	private final Locale DEFAULT_LOCALE = Locale.ENGLISH;
	
	private int id;
	private Date date;
	private Map<String,Message> info = new HashMap<String, NewsItem.Message>();
	private boolean isRead = false;

	public  static class Message {
		private String lang ="";
		private String locale = "";		
		private String title ="";
		private String body ="";

		public String getLang() {
			return lang;
		}
		public void setLang(String lang) {
			this.lang = lang;
		}
		public String getLocale() {
			return locale;
		}
		public void setLocale(String locale) {
			this.locale = locale;
		}

		public String getTitle() {
			return title;
		}
		public void setTitle(String title) {
			this.title = title;
		}
		public String getBody() {
			return body;
		}
		public void setBody(String body) {
			this.body = body;
		}
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public Date getDate() {
		return date;
	}
	public void setDate(Date date) {
		this.date = date;
	}
	
	public void addMessage(Message message) {
		info.put(message.locale.toLowerCase(Locale.ENGLISH),message );
	}

	public Message getMessageByLocale(String locale) {
		Message mess = info.get(locale.toLowerCase(Locale.ENGLISH));
		
		if (mess == null) {
			mess = info.get(DEFAULT_LOCALE.getLanguage());
		}
		if (mess == null && !info.isEmpty()) {
			mess = info.values().iterator().next();
		}
		
		return mess;
	}
	public Collection<String> getAllLocales() {
		return info.keySet();
	}
	
	public boolean isRead() {
		return isRead;
	}
	public void setRead(boolean isRead) {
		this.isRead = isRead;
	}
	
	
	
}
