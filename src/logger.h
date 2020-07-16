#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
class fileLogger;
extern fileLogger logger;
enum class Level { Debug, Info, Warning, Error, Fatal };
class fileLogger {
    class LogStream;

   public:
    fileLogger(std::string filename) noexcept;
    fileLogger(const fileLogger &) = delete;
    fileLogger(fileLogger &&) = delete;

    virtual LogStream operator()(Level nLevel = Level::Debug);

   private:
    const tm *getLocalTime();
    void endline(Level nLevel, std::string &&oMessage);
    void output(const tm *p_tm, const char *str_level, const char *str_message);

   private:
    std::mutex _lock;
    tm _localTime;
    std::ofstream _file;
};
class fileLogger::LogStream : public std::ostringstream {
    fileLogger &m_oLogger;
    Level m_nLevel;

   public:
    LogStream(fileLogger &oLogger, Level nLevel)
        : m_oLogger(oLogger), m_nLevel(nLevel){};
    LogStream(const LogStream &ls)
        : m_oLogger(ls.m_oLogger), m_nLevel(ls.m_nLevel){};
    ~LogStream() { m_oLogger.endline(m_nLevel, std::move(str())); }
};
