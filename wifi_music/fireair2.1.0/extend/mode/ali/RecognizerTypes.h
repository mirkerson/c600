#ifndef DOUG_OS_RECOGNIZERTYPES_H
#define DOUG_OS_RECOGNIZERTYPES_H

#include <string>
#define RECOGNIZER_FRAME_SIZE 320

enum RecognizerResultStatus
{
    SUCCESS,NETWORKING_ERROR,NETWORKING_TIMEOUT,NONE,HAVENOT_STARTED,INTERNAL_ERROR
};

enum RecognizerStartResult
{
    START_SUCCESS,ALREADY_STARTED,UUID_INVALID
};

struct RecognizerResult
{
    RecognizerResultStatus status;
    int alinkStatus;
    std::string asrResult;
    std::string uuid;
    std::string extraMsg;
};

#endif
