#ifndef __SPEECH_CONFIG_H__
#define __SPEECH_CONFIG_H__

#include <iostream>
#include <sstream>

using namespace std;


class SpeechConfig
{
    public:
        SpeechConfig();
        ~SpeechConfig(){};
        int init(const char* file);
        const int GetPort() const;
        const char* GetHost() const;
        const char* GetVersion() const;
        const char* GetSig() const;
        const char* GetLang() const;
        const char* GetRetMod() const;
        const char* GetNlpMod() const;
        const char* GetAsrSC() const;
        const char* GetAppKey() const;
        const char*GetContext() const;
        const char*GetReqUUID() const;
        const bool GetEnableVad() const;
        const bool GetPrefetchDns() const;
        const int GetUseSSL() const;
        const char* GetCaPath() const;
        SpeechConfig& setPort(int port) { std::stringstream out; out<< port; m_port = out.str(); return *this; }
        SpeechConfig& setHost(char *host){ m_host = string(host); return *this;}
        SpeechConfig& setAppKey(char *appKey) { m_appkey = string(appKey); return *this; }
        SpeechConfig& setAsrSC(char *codec) {m_codec = string(codec);return *this;}
        SpeechConfig& setNlpMode(char *mode){m_nlpmode = string(mode);return *this;}
        SpeechConfig& setRetMode(char *mode){m_retmode = string(mode);return *this;}
        SpeechConfig& setLang(char *lang){m_lang = string(lang);return *this;}
        SpeechConfig& setSig(char *sig){m_sig=string(sig);return *this;}
        SpeechConfig& setVersion(char *version){m_ver=string(version);return *this;}
        SpeechConfig& setContext(char *context){ m_context =string(context);return *this;}
        SpeechConfig& setReqUUID(char *req_uuid){ m_req_uuid =string(req_uuid);return *this;}
        SpeechConfig& setEnableVad(bool enableVad){m_enableVad=enableVad;return *this;}
        SpeechConfig& setPrefetchDns(bool prefetchDns){m_prefetchDns=prefetchDns;return *this;}
        SpeechConfig& setUseSSL(int useSSL){m_useSSL = useSSL;return *this;}
        SpeechConfig& setCaPath(char* caPath){m_caPath = caPath;return *this;}
    private:
        string m_host;
        string m_port;
        string m_appkey;
        string m_log;
        string m_codec;
        string m_nlpmode;
        string m_retmode;
        string m_lang;
        string m_sig;
        string m_ver;
        string m_context;
        string m_req_uuid;
        string m_caPath;
        int m_useSSL;
        bool m_enableVad;
        bool m_prefetchDns;
};

#endif
