/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   log.h
    Desc:       just logger
    Created:    by DimaSkup
\**********************************************************************************/
#ifndef LOG_H
#define LOG_H


//---------------------------------------------------------
// constants
//---------------------------------------------------------
#define LOG_BUF_SIZE 512
#define LOG_STORAGE_SIZE 1024
#define LOG_MSGS_CHARS_BUF_SIZE 65536

//---------------------------------------------------------
// it is necessary to differ logs when we print it in the editor's GUI
//---------------------------------------------------------
enum eLogType
{
    LOG_TYPE_MESSAGE,
    LOG_TYPE_DEBUG,
    LOG_TYPE_ERROR,
    LOG_TYPE_FORMATTED,
    LOG_TYPE_FATAL
};

//---------------------------------------------------------
// Desc:   a buffer for all the chars of all the log messages;
//         content of each msg is pushed into this buffer
//         (preferably we may use it in the UI log printing)
//---------------------------------------------------------
struct LogMsgsCharsBuffer
{
    char*     buf      = nullptr;
    const int maxSize  = LOG_MSGS_CHARS_BUF_SIZE;
    int       currSize = 0;
};

//---------------------------------------------------------
// a single log message metadata
//---------------------------------------------------------
struct LogMessage
{
    int startIdx = 0;   // index of the log message beginning in the msgs chars pool
    int size = 0;       // length of the log message + null-terminator

    eLogType type = LOG_TYPE_MESSAGE;
};

//---------------------------------------------------------
// Desc:   arr of log messages and the current number of logs msgs
//---------------------------------------------------------
struct LogStorage
{
    LogMessage logs[LOG_STORAGE_SIZE];
    int        numLogs = 0;
};

//---------------------------------------------------------
// macros to setup console color
//---------------------------------------------------------
#define LOG_RESET       "\033[0m"
#define LOG_BLACK       "\033[30m"              /* Black */
#define LOG_RED         "\033[31m"              /* Red */
#define LOG_GREEN       "\033[32m"              /* Green */
#define LOG_YELLOW      "\033[33m"              /* Yellow */
#define LOG_BLUE        "\033[34m"              /* Blue */
#define LOG_MAGENTA     "\033[35m"              /* Magenta */
#define LOG_CYAN        "\033[36m"              /* Cyan */
#define LOG_WHITE       "\033[37m"              /* White */
#define LOG_BOLDBLACK   "\033[1m\033[30m"       /* Bold Black */
#define LOG_BOLDRED     "\033[1m\033[31m"       /* Bold Red */
#define LOG_BOLDGREEN   "\033[1m\033[32m"       /* Bold Green */
#define LOG_BOLDYELLOW  "\033[1m\033[33m"       /* Bold Yellow */
#define LOG_BOLDBLUE    "\033[1m\033[34m"       /* Bold Blue */
#define LOG_BOLDMAGENTA "\033[1m\033[35m"       /* Bold Magenta */
#define LOG_BOLDCYAN    "\033[1m\033[36m"       /* Bold Cyan */
#define LOG_BOLDWHITE   "\033[1m\033[37m"       /* Bold White */

//---------------------------------------------------------

// macros for standard log message (info about caller: file_name, func_name, code_line, message)
#define LOG __FILE__, __func__, __LINE__

// string global container
extern char g_String[LOG_BUF_SIZE];

//---------------------------------------------------------

int  InitLogger(const char* logFileName);      // call it at the very beginning of the application
void CloseLogger();                            // call it at the very end of the application
void SetConsoleColor(const char* keyColor);

int         GetNumLogMsgs();
const char* GetLogTextByIdx(const int idx);
eLogType    GetLogTypeByIdx(const int idx);


void LogMsg(const char* format, ...);

void LogMsg(
    const char* fileName,
    const char* funcName,
    const int codeLine,
    const char* format,
    ...);

void LogDbg(
    const char* fileName,
    const char* funcName,
    const int codeLine,
    const char* format,
    ...);

void LogErr(
    const char* fileName,
    const char* funcName,
    const int codeLine,
    const char* format,
    ...);

void LogFatal(
    const char* fileName,
    const char* funcName,
    const int codeLine,
    const char* format,
    ...);

#endif