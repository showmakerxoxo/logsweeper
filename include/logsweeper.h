#pragma once

#ifndef LOGSWEEPER_H
#define LOGSWEEPER_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

class LogSweeperImpl;

class LogSweeper {
 public:
  LogSweeper();
  ~LogSweeper();

  void setLogPath(const std::string &in_path);
  void setWaitTime(unsigned int in_time);
  void setLimitSize(unsigned int in_size);
  void start();
  void stop();

 private:
  std::unique_ptr<LogSweeperImpl> m_impl;
};

#endif