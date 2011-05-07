#ifndef __AV_FORMAT_WRAPPER_H__
#define __AV_FORMAT_WRAPPER_H__

extern "C" {
#include <libavformat/avformat.h>
}

#include <string>

class AVFormatWrapper
{
public:
    AVFormatWrapper();
    ~AVFormatWrapper();
    void openFile(const std::string & strFilePath);
private:
    AVFormatContext * m_pFormatCtx;
    static bool s_bInitialized;
    
    void closeFile();
    bool isFileOpened() const;
};

#endif //__AV_FORMAT_WRAPPER_H__
