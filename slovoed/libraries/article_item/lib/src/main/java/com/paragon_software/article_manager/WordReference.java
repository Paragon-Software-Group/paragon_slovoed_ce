package com.paragon_software.article_manager;

public class WordReference {
	private final int mStart;
	private final int mEnd;

	public WordReference(int start, int end) {
		this.mStart = start;
		this.mEnd = end;
	}

	public int getStart() {
		return mStart;
	}

	public int getEnd() {
		return mEnd;
	}
}
