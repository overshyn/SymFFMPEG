#ifndef __AV_FORMAT_WRAPPER_H__
#define __AV_FORMAT_WRAPPER_H__

extern "C" {
#include <libavformat/avformat.h>
}

#include <string>
#include <memory>

class AVFormatWrapper
{
public:
    AVFormatWrapper();
    ~AVFormatWrapper();
    void openFile(const std::string & strFilePath);
    AVPacket * readNextPacket();
private:
    AVFormatContext * m_pFormatCtx;
    std::auto_ptr<AVPacket> m_apAVPacket;
    bool m_bEndOfFile;
    static bool s_bInitialized;
    
    void closeFile();
    bool isFileOpened() const;
    void checkClearPreviousPacket();
};

#endif //__AV_FORMAT_WRAPPER_H__
