#include "include/logsweeper.h"

#include <functional>
#include <atomic>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

class LogSweeperImpl {
 public:
  LogSweeperImpl() {}

  ~LogSweeperImpl() { stop(); }

  void setLogPath(const std::string& in_path);
  void setWaitTime(unsigned int in_time);
  void setLimitSize(unsigned int in_size);
  void start();
  void stop();

 private:
  std::thread m_thread;
  std::mutex m_mutex;
  std::mutex m_runningMutex;
  std::condition_variable m_cv;
  std::atomic_bool m_running{false};
  std::atomic_bool m_stopAtOnce{false};
  std::string m_logPath;
  unsigned int m_waitTime{10};
  /**
   * @brief m_limitSize
   * @note 若为0则不限制数量
   */
  unsigned int m_limitSize{10};

 private:
  /**
   * @brief _sweep
   * @note 保留每个日志级别（INFO、WARNING、ERROR）最近的 m_limitSize 条日志文件
   *      -> log文件夹只会保留 3 * m_limitSize 条文件
   */
  void _sweep();
  void _stopAtOnce();
  std::vector<std::string> _getFilesInDir(const std::string& in_dirPath);
};

void LogSweeperImpl::setLogPath(const std::string& in_path) {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_logPath = in_path;
}

void LogSweeperImpl::setWaitTime(unsigned int in_time) {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_waitTime = in_time;
}

void LogSweeperImpl::setLimitSize(unsigned int in_size) {
  std::unique_lock<std::mutex> lock(m_mutex);
  m_limitSize = in_size;
}

void LogSweeperImpl::start() {
  m_running.store(true);
  if (!m_thread.joinable()) {
    m_thread = std::thread(std::bind(&LogSweeperImpl::_sweep, this));
  }
}

void LogSweeperImpl::stop() { _stopAtOnce(); }

void LogSweeperImpl::_sweep() {
  try {
    std::unique_lock<std::mutex> lock(m_runningMutex);
    while (m_running.load()) {
      m_cv.wait_for(lock, std::chrono::seconds(m_waitTime));

      if (m_stopAtOnce.load()) {
        break;
      }

      std::unique_lock<std::mutex> temp(m_mutex);
      auto list = _getFilesInDir(m_logPath);
      unsigned int limitSize = m_limitSize;
      temp.unlock();

      std::vector<std::string> infoFiles, warningFiles, errorFiles;
      for (const auto& file : list) {
        if (file.find("INFO") != std::string::npos) {
          infoFiles.push_back(file);
        } else if (file.find("WARNING") != std::string::npos) {
          warningFiles.push_back(file);
        } else if (file.find("ERROR") != std::string::npos) {
          errorFiles.push_back(file);
        }
      }

      auto sortFiles = [](std::vector<std::string>& files) {
        std::sort(files.begin(), files.end(),
                  [](const std::string& a, const std::string& b) {
          struct stat fa_stat, fb_stat;
          stat(a.c_str(), &fa_stat);
          stat(b.c_str(), &fb_stat);

          if (fa_stat.st_mtim.tv_sec > fb_stat.st_mtim.tv_sec) {
            return true;
          } else if (fa_stat.st_mtim.tv_sec == fb_stat.st_mtim.tv_sec) {
            return fa_stat.st_mtim.tv_nsec > fb_stat.st_mtim.tv_nsec;
          } else {
            return false;
          }
        });
      };

      sortFiles(infoFiles);
      sortFiles(warningFiles);
      sortFiles(errorFiles);

      auto deleteExtraFiles = [](std::vector<std::string>& files,
                                 unsigned int limit) {
        if (limit == 0) {
          return;
        }

        if (files.empty()) {
          return;
        }

        if (files.size() > limit) {
          for (int i = limit; i < files.size(); ++i) {
            int ret = std::remove(files[i].c_str());

            if (ret == 0) {
              std::cout << "removed:" << files[i].c_str() << std::endl;
            } else {
              std::cerr << "failed to remove: " << files[i].c_str()
                        << " | error: " << strerror(errno) << std::endl;
            }
          }
        }
      };

      deleteExtraFiles(infoFiles, limitSize);
      deleteExtraFiles(warningFiles, limitSize);
      deleteExtraFiles(errorFiles, limitSize);
    }
  }
  catch (const std::exception& e) {
  }
}

void LogSweeperImpl::_stopAtOnce() {
  m_stopAtOnce.store(true);
  m_running.store(false);
  m_cv.notify_all();
  if (m_thread.joinable()) {
    m_thread.join();
  }
}

std::vector<std::string> LogSweeperImpl::_getFilesInDir(
    const std::string& in_dirPath) {
  std::vector<std::string> files;

#ifdef _WIN32
#else
  /// linux/macos
  DIR* dir = opendir(in_dirPath.c_str());
  if (!dir) {
    return files;
  }

  struct dirent* entry;
  while ((entry = readdir(dir))) {
    std::string filename = entry->d_name;

    /// skip special directories
    if (filename == "." || filename == "..") {
      continue;
    }

    std::string fullPath = in_dirPath + "/" + filename;
    struct stat statbuf;
    if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
      /// only normal files
      files.push_back(fullPath);
    }
  }
  closedir(dir);
#endif

  return files;
}

LogSweeper::LogSweeper() : m_impl(new LogSweeperImpl()) {}

LogSweeper::~LogSweeper() {}

void LogSweeper::setLogPath(const std::string& in_path) {
  m_impl->setLogPath(in_path);
}

void LogSweeper::setWaitTime(unsigned int in_time) {
  m_impl->setWaitTime(in_time);
}

void LogSweeper::setLimitSize(unsigned int in_size) {
  m_impl->setLimitSize(in_size);
}

void LogSweeper::start() { m_impl->start(); }

void LogSweeper::stop() { m_impl->stop(); }
