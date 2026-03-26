#pragma once
#include <string>
#include <deque>
#include <mutex>

enum class LogType
{
  Error,
  Log,
};

struct LogItem
{
  std::string message;
  LogType logType;
};

extern std::mutex logMutex;
extern std::deque<LogItem> logHistory;