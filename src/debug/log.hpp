#ifndef EMP_LOG_H
#define EMP_LOG_H
#include <stdio.h>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace emp {

std::string NowTime();

// enum TLogLevel {logERROR, logWARNING, logINFO, logDEBUG, logDEBUG1,
// logDEBUG2, logDEBUG3, logDEBUG4};
enum LogLevel { ERROR, WARNING, INFO, DEBUG, DEBUG1, DEBUG2, DEBUG3 };

class LogOutput {
public:
    virtual void Output(std::string msg) = 0;
    virtual ~LogOutput() {
    }
};

class Log {
public:
    Log();
    virtual ~Log();
    std::ostringstream& Get(LogLevel level = INFO);

public:
    static std::unique_ptr<LogOutput> s_out;
    static std::mutex s_out_lock;

    static LogLevel s_reporting_level;
    static std::string ToString(LogLevel level);

protected:
    std::ostringstream os;

private:
    Log(const Log&);
    Log& operator=(const Log&);
};

class Output2Cerr : public LogOutput {
public:
    void Output(std::string msg) override {
        std::cerr << msg;
    }
};

class Output2FILE : public LogOutput {
    std::ofstream file;

public:
    void Output(std::string msg) override {
        auto begin_esc = msg.find('\033');
        while (begin_esc != std::string::npos) {
            auto end_esc = msg.find('m', begin_esc);
            assert(end_esc != std::string::npos);
            msg.erase(begin_esc, end_esc - begin_esc);
            begin_esc = msg.find('\033');
        }
        file << msg;
    }
    Output2FILE(std::string filename) : file(filename) {
    }
};

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL DEBUG3
#endif

#define EMP_LOG(level)                       \
    if (level > FILELOG_MAX_LEVEL)           \
        ;                                    \
    else if (level > Log::s_reporting_level) \
        ;                                    \
    else                                     \
        Log().Get(level)

#define EMP_LOG_DEBUG                                  \
    if (LogLevel::DEBUG > FILELOG_MAX_LEVEL)           \
        ;                                              \
    else if (LogLevel::DEBUG > Log::s_reporting_level) \
        ;                                              \
    else                                               \
        Log().Get(LogLevel::DEBUG)

#define EMP_LOG_TRACE(level)                          \
    if (level > FILELOG_MAX_LEVEL)                    \
        ;                                             \
    else if (level > Log::s_reporting_level)          \
        ;                                             \
    else                                              \
        Log().Get(level) << "{ in file: " << __FILE__ \
                         << ", at line: " << __LINE__ << " }:"

#define EMP__HELPER_CONCAT_(prefix, suffix) prefix##suffix
#define EMP__HELPER_CONCAT(prefix, suffix) EMP__HELPER_CONCAT_(prefix, suffix)
#define EMP__HELPER_MAKE_UNIQUE_VARIABLE_NAME(prefix) \
    EMP__HELPER_CONCAT(prefix##_, __LINE__)

#define EMP__HELPER_LOG_INTERVAL(level, time, uniquetp, uniquecanlog)     \
    static std::chrono::time_point<std::chrono::high_resolution_clock>    \
            uniquetp = std::chrono::high_resolution_clock::now();         \
    static int uniquecanlog = 1;                                          \
    if (level > FILELOG_MAX_LEVEL)                                        \
        ;                                                                 \
    else if (level > Log::s_reporting_level)                              \
        ;                                                                 \
    else if (std::chrono::duration<float, std::chrono::seconds::period>(  \
                     std::chrono::high_resolution_clock::now() - uniquetp \
             )                                                            \
                     .count() >= time) {                                  \
        uniquetp = std::chrono::high_resolution_clock::now();             \
        uniquecanlog = 1;                                                 \
    }                                                                     \
    if (uniquecanlog != 0 && uniquecanlog-- == 1)                         \
    Log().Get(level)

#define EMP_LOG_INTERVAL(level, time)                       \
    EMP__HELPER_LOG_INTERVAL(                               \
            level,                                          \
            time,                                           \
            EMP__HELPER_MAKE_UNIQUE_VARIABLE_NAME(          \
                    time_point_name_that_you_will_never_use \
            ),                                              \
            EMP__HELPER_MAKE_UNIQUE_VARIABLE_NAME(          \
                    canlog_name_that_you_will_never_use     \
            )                                               \
    )

//#define L_(level) \
//if (level > FILELOG_MAX_LEVEL) ;\
//else if (level > FILELog::s_reporting_level || !Output2FILE::Stream()) ; \
//else FILELog().Get(level)

} // namespace emp
#endif // emp_LOG_H
