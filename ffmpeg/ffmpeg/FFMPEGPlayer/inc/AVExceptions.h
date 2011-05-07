#ifndef __AV_EXCEPTIONS_H__
#define __AV_EXCEPTIONS_H__

#include <exception>

class AVBaseException: public std::exception
{
public:
    explicit AVBaseException(const std::string & strMessage): m_strMessage(strMessage) {}
    virtual const char* what() const { return m_strMessage.c_str(); }
protected:
    std::string m_strMessage;
};

class AVOpenFileException: public AVBaseException
{
public:
    AVOpenFileException(): AVBaseException("Can't open file!") {}
};

class AVObtainStreamsException: AVBaseException
{
public:
    AVObtainStreamsException(): AVBaseException("Can't retreive streams!") {}
};

#endif //__AV_EXCEPTIONS_H__
