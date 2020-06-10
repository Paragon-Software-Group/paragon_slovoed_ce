package com.paragon_software.engine.rx;

public interface Cancelable
{
  void cancel();

//  void complete();

  boolean isCanceled();

//  boolean isComplete();
}
