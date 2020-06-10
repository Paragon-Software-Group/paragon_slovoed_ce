package com.paragon_software.search_manager;

public interface OnErrorListener extends SearchManagerAPI.Notifier
{
  void onSearchManagerError( Exception exception );
}
