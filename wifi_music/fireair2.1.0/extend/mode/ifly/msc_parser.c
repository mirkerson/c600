
#ifdef __IFLY_VOICE__

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"

#include "json.h"
#include "ifly.h"


static int ifly_busy = 0;

char str_weather[2048] = "";


P_LIST_URL url_list = NULL;

P_LIST_URL create_url_list(int n, array_list* arr, char* str)
{
	int i = 0;
	P_LIST_URL head_pk = NULL;
	P_LIST_URL new_pk = NULL;
	P_LIST_URL tmp_pk = NULL;

    json_object* arr_obj;
    json_object *obj_url;
    
    int len;

    head_pk = (P_LIST_URL)malloc(sizeof(S_LIST_URL));
    if(head_pk == NULL)
        return NULL;

    head_pk->url = NULL;
    
    arr_obj = (json_object*)array_list_get_idx(arr,i); 

    obj_url = json_object_object_get(arr_obj, str);
    if(NULL != obj_url)
    {
        len = strlen(json_object_get_string(obj_url));
        if(len > 0)
        {
            head_pk->url = (char *)malloc(len + 1);    
            strcpy(head_pk->url, json_object_get_string(obj_url));
        }

    }
    
    head_pk->next = NULL;
    head_pk->prev = NULL;

	tmp_pk = head_pk;
    
	for(i=1; i<n; i++){
        
		new_pk = (P_LIST_URL)malloc(sizeof(S_LIST_URL));
		if(new_pk == NULL)
			return NULL;

        new_pk->url = NULL;
        
        arr_obj = (json_object*)array_list_get_idx(arr,i); 

        obj_url = json_object_object_get(arr_obj, str);
        if(NULL != obj_url)
        {
            len = strlen(json_object_get_string(obj_url));
            if(len > 0)
            {
                new_pk->url = (char *)malloc(len + 1);    
                strcpy(new_pk->url, json_object_get_string(obj_url));
            }

        }

 		tmp_pk->next = new_pk;
        new_pk->prev = tmp_pk;

		new_pk->next = NULL;
        
        
		tmp_pk = new_pk;
        
	}

	tmp_pk->next = head_pk;
    head_pk->prev = tmp_pk;

    return head_pk;
    
}

void delete_url_list(void)
{
	P_LIST_URL tmp_pk1;
	P_LIST_URL tmp_pk2;

    if(NULL == url_list)
        return;

    tmp_pk1 = url_list->next;
    tmp_pk2 = tmp_pk1->next;

	while(tmp_pk1 != url_list)
	{
	    if(NULL != tmp_pk1->url)
	        free(tmp_pk1->url);
		free(tmp_pk1);
		tmp_pk1 = tmp_pk2;
		tmp_pk2 = tmp_pk2->next;
	}

    if(NULL != url_list->url)
        free(url_list->url);
	free(url_list);
    
	url_list = NULL;
}


int demux_json_content(char *str)
{
    const char* param2 = "vcn=xiaoyan,aue = speex-wb,auf=audio/L16;rate=16000,spd = 5,vol = 5,tte = utf8";//8k音频合成参数：aue=speex,auf=audio/L16;rate=8000,其他参数意义参考参数列表

    json_object *my_obj;
    int ret;    
    int len;
    char cmd[128];

    delete_url_list();

    my_obj = json_tokener_parse(str);
    
    if(NULL == my_obj ) {	
    	printf("json_tokener_parse string failed!\n");
    	return VOICE_UNKNOW;
    }

    
    json_object *obj_data = json_object_object_get(my_obj, "data");
    if(NULL == obj_data)
        return VOICE_UNKNOW;


    int i=0,j;

    json_object_object_foreach(obj_data,key,val)
    {

        if (strcmp(key,"result") == 0)
        {
            if( (len = json_object_array_length(val)) > 0)
            {
                array_list* arr = json_object_get_array(val);
                if(NULL == arr)
                    return VOICE_UNKNOW;

                printf("create_url_list %d node!\n", len);
                
                url_list = create_url_list(len, arr, "downloadUrl");
                
                for(i = 0; i < len; i++)
                {
                    printf("url_list[%d]=%s\n", i, url_list->url);
                    
                    if(NULL != url_list->url)
                    {
                        //mifly_obj->vptr->stop();
                        //mifly_obj->vptr->play(url_list->url);
                        
                        json_object_put(my_obj);        
                        return VOICE_MUSIC;
                    }     
                    
                    url_list = url_list->next;
                }

                    delete_url_list();
                        
                    json_object* arr_obj = (json_object*)array_list_get_idx(arr,0);     
                    if(NULL == arr_obj)
                        return VOICE_UNKNOW;
                    
                    json_object *obj_weather[7];
                    obj_weather[2] = json_object_object_get(arr_obj, "weather");
                    if(NULL != obj_weather[2])
                    {
                        obj_weather[0] = json_object_object_get(arr_obj, "city");
                        obj_weather[1] = json_object_object_get(arr_obj, "date");                    
                        obj_weather[3] = json_object_object_get(arr_obj, "tempRange");
                        obj_weather[4] = json_object_object_get(arr_obj, "wind");
                        obj_weather[5] = json_object_object_get(arr_obj, "windLevel");
                        obj_weather[6] = json_object_object_get(arr_obj, "airQuality");

                        for(j=0;j<7;j++)
                        {
                            //printf("%s ", json_object_get_string(obj_weather[j]));
                            if(j == 5)
                                strcat(str_weather, "风力"); 
                            else if(j == 6)
                                strcat(str_weather, "空气质量"); 

                            strcat(str_weather, json_object_get_string(obj_weather[j]));

                            if(j == 5)
                                strcat(str_weather, "级");
                            if(j != 6)
                                strcat(str_weather, "，");
                            else
                                strcat(str_weather, "。");
                        }

                        printf("str_weather: %s\n", str_weather);

                        json_object_put(my_obj);  

                        sprintf(cmd,"rm \"%s\"", REC_FILE);    
                        system(cmd);   

                        ret = text_to_speech(str_weather, TTS_FILE, param2);
                        if(0 == ret)
                        {
                            return VOICE_WEATHER;   
                        }
                        else
                            return VOICE_UNKNOW;
                        
                    }
                        
                    json_object_put(my_obj);
                    return VOICE_UNKNOW;

            }
            else
            {
                json_object_put(my_obj);
                return VOICE_UNKNOW;
            }
        }
    }

}


int run_iat(const char* src_wav_filename, const char* param)
{
	char rec_result[1024*16] = "";
	const char *sessionID;
	FILE *f_pcm = NULL;
	char *pPCM = NULL;
	int audStat = 2 ;
	int epStatus = 0;
	int recStatus = 0 ;
	long pcmCount = 0;
	long pcmSize = 0;
	int ret = 0 ;
    
    
	sessionID = QISRSessionBegin(NULL, param, &ret);
    if (ret !=0)
	{
	    printf("QISRSessionBegin Failed,ret=%d\n",ret);
        return VOICE_UNKNOW;
	}
	f_pcm = fopen(src_wav_filename, "rb");
	if(NULL == f_pcm)
	{
        printf("no such file\n");
        return VOICE_UNKNOW;
    }
		
    
	if (NULL != f_pcm) {
		fseek(f_pcm, 0, SEEK_END);
		pcmSize = ftell(f_pcm);
        if(pcmSize <= 0)
        {
            fclose(f_pcm);
            return VOICE_UNKNOW;
        }
            
        
		fseek(f_pcm, 0, SEEK_SET);
        
		pPCM = (char *)malloc(pcmSize);
        if(NULL == pPCM)
        {
            fclose(f_pcm);
            return VOICE_UNKNOW;
        }

        
		fread((void *)pPCM, pcmSize, 1, f_pcm);
		fclose(f_pcm);
		f_pcm = NULL;
	}
	while (1) {
		unsigned int len = 6400;
        unsigned int audio_len = 6400;
		if (pcmSize < 12800) {
			len = pcmSize;
		}
		audStat = 2;
		if (pcmCount == 0)
			audStat = 1;
		if (0) {
			if (audStat == 1)
				audStat = 5;
			else
				audStat = 4;
		}

		printf("\ncsid=%s,count=%d,aus=%d,",sessionID,pcmCount/audio_len,audStat);
		ret = QISRAudioWrite(sessionID, (const void *)&pPCM[pcmCount], len, audStat, &epStatus, &recStatus);
		printf("eps=%d,rss=%d,ret=%d",epStatus,recStatus,ret);
		if (ret != 0)
			break;
		pcmCount += (long)len;
		pcmSize -= (long)len;
		if (recStatus == 0) {
			const char *rslt = QISRGetResult(sessionID, &recStatus, 0, &ret);
			if (ret != MSP_SUCCESS)
			{
				printf("QISRGetResult Failed,ret=%d\n",ret);
				break;
			}
			if (NULL != rslt)
				strcat(rec_result,rslt);
		}
		if (epStatus == MSP_EP_AFTER_SPEECH)
			break;
		usleep(150000);
	}
	ret=QISRAudioWrite(sessionID, (const void *)NULL, 0, 4, &epStatus, &recStatus);
	if (ret !=0)
	{
		printf("QISRAudioWrite Failed,ret=%d\n",ret);
	}
	free(pPCM);
	pPCM = NULL;
	while (recStatus != 5 && ret == 0) {
		const char *rslt = QISRGetResult(sessionID, &recStatus, 0, &ret);
		if (NULL != rslt)
		{
			strcat(rec_result,rslt);
		}
		usleep(150000);
	}
    ret=QISRSessionEnd(sessionID, NULL);
	if(ret !=MSP_SUCCESS)
	{
		printf("QISRSessionEnd Failed, ret=%d\n",ret);
	}

	printf("\n=============================================================\n");
	printf("The result is: %s\n",rec_result);
	printf("=============================================================\n");
	usleep(100);
       
    ret = demux_json_content(rec_result);
    if(VOICE_UNKNOW == ret)
    {
        ffc_inst.player.tone_play("/home/ad/searchfail.mp3", 0);          
    }

    return ret;
 //   MSPLogout();
	
}


typedef int SR_DWORD;
typedef short int SR_WORD ;

//音频头部格式
struct wave_pcm_hdr
{
	char            riff[4];                        // = "RIFF"
	SR_DWORD        size_8;                         // = FileSize - 8
	char            wave[4];                        // = "WAVE"
	char            fmt[4];                         // = "fmt "
	SR_DWORD        dwFmtSize;                      // = 下一个结构体的大小 : 16

	SR_WORD         format_tag;              // = PCM : 1
	SR_WORD         channels;                       // = 通道数 : 1
	SR_DWORD        samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	SR_DWORD        avg_bytes_per_sec;      // = 每秒字节数 : dwSamplesPerSec * wBitsPerSample / 8
	SR_WORD         block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	SR_WORD         bits_per_sample;         // = 量化比特数: 8 | 16

	char            data[4];                        // = "data";
	SR_DWORD        data_size;                // = 纯数据长度 : FileSize - 44 
} ;

//默认音频头部数据
struct wave_pcm_hdr default_pcmwavhdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};

int text_to_speech(const char* src_text ,const char* des_path ,const char* params)
{
	struct wave_pcm_hdr pcmwavhdr = default_pcmwavhdr;
	const char* sess_id = NULL;
	int ret = 0;
	unsigned int text_len = 0;
	unsigned int audio_len = 0;
	int synth_status = 1;
	FILE* fp = NULL;

	printf("\nbegin to synth...\n");
	if (NULL == src_text || NULL == des_path)
	{
		printf("params is null!\n");
		return -1;
	}
	text_len = (unsigned int)strlen(src_text);
	fp = fopen(des_path,"wb");
	if (NULL == fp)
	{
		printf("open file %s error\n",des_path);
		return -1;
	}
	sess_id = QTTSSessionBegin(params, &ret);
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSSessionBegin: qtts begin session failed Error code %d.\n",ret);
        fclose(fp);
		return ret;
	}

	ret = QTTSTextPut(sess_id, src_text, text_len, NULL );
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSTextPut: qtts put text failed Error code %d.\n",ret);
		QTTSSessionEnd(sess_id, "TextPutError");
        fclose(fp);
		return ret;
	}
	fwrite(&pcmwavhdr, sizeof(pcmwavhdr) ,1, fp);
	while (1) 
	{
		const void *data = QTTSAudioGet(sess_id, &audio_len, &synth_status, &ret);
		if (NULL != data)
		{
		   fwrite(data, audio_len, 1, fp);
		   pcmwavhdr.data_size += audio_len;//修正pcm数据的大小
		}
              usleep(150000);//建议可以sleep下，因为只有云端有音频合成数据，audioget都能获取到音频。
		if (synth_status == 2 || ret != 0) 
		break;
	}

	//修正pcm文件头数据的大小
	pcmwavhdr.size_8 += pcmwavhdr.data_size + 36;

	//将修正过的数据写回文件头部
	fseek(fp, 4, 0);
	fwrite(&pcmwavhdr.size_8,sizeof(pcmwavhdr.size_8), 1, fp);
	fseek(fp, 40, 0);
	fwrite(&pcmwavhdr.data_size,sizeof(pcmwavhdr.data_size), 1, fp);
	fclose(fp);

	ret = QTTSSessionEnd(sess_id, NULL);
	if ( ret != MSP_SUCCESS )
	{
	printf("QTTSSessionEnd: qtts end failed Error code %d.\n",ret);
	}
       printf("\nTTS end...\n");
	return ret;
}



int demo_voice_kdxf(const char* src_wav_filename)
{
	const char* param1 = "nlp_version = 2.0, sch = 1, sub = iat, domain = music, language = zh_ch, accent = mandarin, sample_rate = 16000, result_type = json, result_encoding = utf8";
    const char* login_configs = "appid = 55cb0ee3, work_dir =   .  ";//5608d33f
    int ret;

    if(ifly_busy)
        return VOICE_UNKNOW;

    ifly_busy = 1;

    ffc_inst.player.tone_play("/home/ad/searching.mp3", 0);  
    
    ret = MSPLogin(NULL, NULL, login_configs);
	if ( ret != MSP_SUCCESS )
	{
		printf("MSPLogin failed , Error code %d.\n",ret);
        return;
	}

    strcpy(str_weather, "");    
        
    ret = run_iat(src_wav_filename, param1);  
        
    MSPLogout();

    ifly_busy = 0;

    return ret;
    
}


#endif

