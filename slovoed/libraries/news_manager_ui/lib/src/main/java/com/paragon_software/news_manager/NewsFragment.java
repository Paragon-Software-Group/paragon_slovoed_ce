package com.paragon_software.news_manager;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.swiperefreshlayout.widget.SwipeRefreshLayout;
import android.text.Html;
import android.text.format.DateFormat;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.paragon_software.news_manager_ui.R;

import java.util.Date;
import java.util.List;


public class NewsFragment extends Fragment
		implements
		AdapterView.OnItemClickListener,
		OnControllerNewsListChangedListener,
		OnNewsContentStateListener,
		SwipeRefreshLayout.OnRefreshListener,
		AbsListView.OnScrollListener {


	private static final long TIMEOUT_PULLING_NEWS = 3500;
	private SwipeRefreshLayout mRefreshLayout;
	private AdapterNewsTitle mAdapterUnReadNews;
	private AdapterNewsTitle mAdapterReadNews;
	private NewsControllerAPI mController;

	private ListView mListView;
	private ListView mListReadView;
	private WebView mContentWebView;

	@Override
	public void onCreate(@Nullable Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setRetainInstance(true);
	}

	@Override
	public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		setHasOptionsMenu(true);
		if (NewsManagerHolder.getManager() != null) {
			mController = NewsManagerHolder.getManager().getController(NewsControllerType.DEFAULT_CONTROLLER);
		}
		return inflater.inflate(R.layout.news_view, container, false);
	}

	@Override
	public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
		super.onViewCreated(view, savedInstanceState);
		mRefreshLayout = view.findViewById(R.id.refresh);
		mRefreshLayout.setOnRefreshListener(this);
		mListView = view.findViewById(R.id.list_news);
		mListReadView = view.findViewById(R.id.list_read_news);
		mListView.setOnItemClickListener(this);
		mListReadView.setOnItemClickListener(this);
		mListView.setOnScrollListener(this);
		mListReadView.setOnScrollListener(this);
		mContentWebView = view.findViewById(R.id.news_webview);
		mContentWebView.setWebViewClient(new ContentNewsWebClient());
		initListNews();
	}

	@Override
	public void onPause() {
		super.onPause();
		mController.unregisterNotifier(this);
	}


	@Override
	public void onResume() {
		super.onResume();
		mController.registerNotifier(this);
		checkBackClick();
		mController.showNewsContent(mController.isContentVisibility());
	}

	private void checkBackClick() {
		if (getView() != null) {
			getView().setFocusableInTouchMode(true);
			getView().requestFocus();
			getView().setOnKeyListener(new View.OnKeyListener() {
				@Override
				public boolean onKey(View v, int keyCode, KeyEvent event) {
					if (keyCode == KeyEvent.KEYCODE_BACK && mContentWebView.getVisibility() == View.VISIBLE) {
						mController.showNewsContent(false);
						return true;
					}
					return false;
				}
			});
		}
	}

	public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
		inflater.inflate(R.menu.news_menu, menu);
		super.onCreateOptionsMenu(menu, inflater);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		if (item.getItemId() == R.id.read_all_news_action) {
			mController.markAllAsRead();
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		NewsItem newsItem = ((AdapterNewsTitle.Holder) view.getTag()).getNewsItem();
		mController.openNewsItem(newsItem);
	}

	private void updateContentVisibility(boolean contentVisibility) {
		mContentWebView.setVisibility(contentVisibility ? View.VISIBLE : View.GONE);
		mRefreshLayout.setVisibility(contentVisibility ? View.GONE : View.VISIBLE);
		setMenuVisibility(!contentVisibility);
	}

	@Override
	public void onNewsContentStateListener(boolean visibility) {
		if (visibility) {
			String html = "<html><style>/*font*/</style><body>" + mController.getShowItem().getBody() + "</body></html>";
			mContentWebView.loadDataWithBaseURL("fake://", html, "text/html", "UTF-8", null);
		} else {
			mContentWebView.loadUrl("about:blank");
			mContentWebView.clearCache(true);
		}
		updateContentVisibility(visibility);
	}

	class ContentNewsWebClient extends WebViewClient {
		@Override
		public boolean shouldOverrideUrlLoading(WebView view, String url) {
			startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(url)));
			return true;
		}
	}

	private void updateListVisibility() {
		mListView.setVisibility(mAdapterUnReadNews.getCount() == 0 ? View.GONE : View.VISIBLE);
		mListReadView.setVisibility(mAdapterReadNews.getCount() == 0 ? View.GONE : View.VISIBLE);
	}

	private void initListNews() {
		mRefreshLayout.setRefreshing(false);
		mAdapterUnReadNews = new AdapterNewsTitle(getContext(), mController.getUnReadNews());
		mAdapterReadNews = new AdapterNewsTitle(getContext(), mController.getReadNews());

		mListView.setAdapter(mAdapterUnReadNews);
		mListReadView.setAdapter(mAdapterReadNews);

		updateListVisibility();
	}

	@Override
	public void onControllerNewsListChanged() {
		mAdapterUnReadNews.renewAdapterData(mController.getUnReadNews());
		mAdapterReadNews.renewAdapterData(mController.getReadNews());
		updateListVisibility();
	}

	class AdapterNewsTitle extends BaseAdapter {

		private List<NewsItem> mNewsList;
		private final Context mContext;
		private final java.text.DateFormat mDateFormat;

		AdapterNewsTitle(Context context, List<NewsItem> newsList) {
			mNewsList = newsList;
			mContext = context;
			mDateFormat = DateFormat.getDateFormat(mContext);
		}

		@Override
		public int getCount() {
			return mNewsList.size();
		}

		@Override
		public NewsItem getItem(int position) {
			return mNewsList.get(position);
		}

		@Override
		public long getItemId(int position) {
			return getItem(position).getId();
		}

		class Holder {

			final TextView textViewDate;
			final TextView textViewTitle;
			final View mView;
			private final TextView textViewContent;
			private NewsItem mNewsItem;

			Holder(View view) {
				mView = view;
				textViewDate = view.findViewById(R.id.item_news_date);
				textViewTitle = view.findViewById(R.id.item_news_title);
				textViewContent = view.findViewById(R.id.item_news_content);
			}

			void applyReadAndSelectedStatus(boolean read) {
				if (read)
					mView.setAlpha(.5f);
				else
					mView.setAlpha(1f);
			}

			void setNewsItem(NewsItem newsItem) {
				mNewsItem = newsItem;
			}

			NewsItem getNewsItem() {
				return mNewsItem;
			}

		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			Holder holder;
			NewsItem newsItem = getItem(position);
			if (convertView == null) {
				LayoutInflater inflater = LayoutInflater.from(mContext);
				View viewItem = inflater.inflate(R.layout.news_item, parent, false);
				holder = new Holder(viewItem);
			} else {
				holder = (Holder) convertView.getTag();
			}
			holder.textViewTitle.setText(Html.fromHtml(newsItem.getTitle()));
			if (!newsItem.isRead()) {
				holder.textViewDate.setText(mDateFormat.format(new Date(newsItem.getDate())));
			}

			holder.textViewContent.setText(Html.fromHtml(newsItem.getBody()));
			holder.applyReadAndSelectedStatus(newsItem.isRead());
			holder.setNewsItem(newsItem);
			holder.mView.setTag(holder);
			return holder.mView;
		}

		void renewAdapterData(List<NewsItem> newsList) {
			mNewsList = newsList;
			notifyDataSetChanged();
		}
	}

	@Override
	public void onScrollStateChanged(AbsListView absListView, int i) {
	}

	@Override
	public void onScroll(AbsListView absListView, int i, int i1, int i2) {
		if (absListView.getChildAt(0) != null && absListView.getChildAt(0).getTop() == 0) {
			mRefreshLayout.setEnabled(true);
		} else {
			mRefreshLayout.setEnabled(false);
		}
	}

	@Override
	public void onRefresh() {
		mController.refreshNews();

		mRefreshLayout.setRefreshing(true);
		mRefreshLayout.postDelayed(new Runnable() {
			@Override
			public void run() {
				if (mRefreshLayout != null)
					mRefreshLayout.setRefreshing(false);
			}
		}, TIMEOUT_PULLING_NEWS);
	}
}

