//
// Created by yinshang on 15-6-9.
//

#ifndef DOUG_OS_RECOGNIZER_H
#define DOUG_OS_RECOGNIZER_H

#include "speechconfig.h"
#include "VadStatus.h"
#include "RecognizerTypes.h"
#include <string>
#include <fstream>

class OpuCodecProxy;
class VoiceSender;
class VoiceActivityDetectorInterface;

class Recognizer {
public:
    Recognizer(): mVoiceSenderPointer(NULL), mOpuCodecProxyPointer(NULL),
                 mVoiceActivityDetectorPointer(NULL), mCurrentVadStatus(SILENCE), mSaveVadFile(false),
                 mVadFilePointer(NULL),mSaveVadFileSequence(0){};
    ~Recognizer();
    bool init(const SpeechConfig config);
    RecognizerStartResult start();
    RecognizerResult getResult();
    RecognizerResult stop();
    void cancel();
    void saveVadFile(std::string path);
    VadStatus update(short pcm[],int length);

private:
    void sendPCM(short *pcm,int length);
    void cleanStagingResources();
    VoiceSender *mVoiceSenderPointer;
    OpuCodecProxy *mOpuCodecProxyPointer;
    VoiceActivityDetectorInterface *mVoiceActivityDetectorPointer;
    VadStatus mCurrentVadStatus;
    SpeechConfig mSpeechConfig;
    bool mSaveVadFile;
    std::string mVadFilePath;
    std::ofstream *mVadFilePointer;
    int mSaveVadFileSequence;
};
#endif //DOUG_OS_RECOGNIZER_H
