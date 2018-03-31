//
// Created by yinshang on 15-5-25.
//
#ifdef __ALI_MUSIC__

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "speechconfig.h"
#include "Recognizer.h"
#include "RecognizerTypes.h"
#include "ali_music.h"

Recognizer *recognizer = NULL;


int singleRecognize(char *fileName,Recognizer *recognizer) 
{
    FILE *pcmFile;
    short samples[RECOGNIZER_FRAME_SIZE];

	
    pcmFile = fopen(fileName,"r");
    if(pcmFile == 0){
        printf("open pcm file failed.\n");
        return -1;
    }

    //recognizer->start();
    RecognizerStartResult startResult = recognizer->start();
    if(startResult != START_SUCCESS){
        printf("start failed, code is %d\n",startResult);
		fclose(pcmFile);
        return -1;
    }
	
    VadStatus  vadStatus;
    while( fread(samples,1,RECOGNIZER_FRAME_SIZE*2,pcmFile) == RECOGNIZER_FRAME_SIZE*2)
    {
        vadStatus = recognizer->update((short *)samples,RECOGNIZER_FRAME_SIZE);
        if(vadStatus == INVALID) {
            std::cout << "update invalid" << std::endl;
        }
    }

    RecognizerResult recognizerResult = recognizer->stop(); //send finish signal 0000
    if(recognizerResult.status == SUCCESS) {
        const char *asr_ret = recognizerResult.asrResult.c_str();
        printf("asr result is %s\n", asr_ret);
		
        const char *uuid = recognizerResult.uuid.c_str();
        printf("main final UUID is %s\n", uuid);
		
		fclose(pcmFile);
		//return recognizerResult.alinkStatus;
		return 0;
    } else {
        printf("get result failed, the status is:%d\n", recognizerResult.status);
		fclose(pcmFile);
		return recognizerResult.status;
    }
    
}


extern "C" int ali_voice_init();

int ali_voice_init()
{
    int ret;
    SpeechConfig speechConfig;
	
    recognizer = new Recognizer();

	
    if(speechConfig.init("/tmp/atalk/sdk-config.json") != 0 ) {
        std::cout << "read sdk-config file failed" << std::endl;
		
		FILE *pcmFile;
        pcmFile = fopen("/tmp/atalk/sdk-config.json","r");
	    if(pcmFile == 0){
	        printf("open sdk-config.json failed.\n");
	        return -1;
	    }else{
			fclose(pcmFile);
		}
        return -1;
    }
    recognizer->init(speechConfig);
	
    return 0;
}


extern "C" int analyze_voice_end(void);

int analyze_voice_end(void)
{
   printf("user_voice_end\n");
   if(recognizer!=NULL){
       delete recognizer;
   }
   return 0;
}

extern "C" int analyze_ali_voice(void);

int analyze_ali_voice(void)
{
	int ret = 0;

	ret = ali_voice_init();
	if(ret < 0){
		printf("ali_voice_init fail !!!\n");
		return  ret;
	}else{
		printf("ali_voice_init success\n");
	}

	ret = singleRecognize(REC_ALI_FILE, recognizer);
	
	printf("ret = %d\n",ret);
	
    analyze_voice_end();

	
	return ret;
}

#endif

