#ifndef _FIREAIR_LIB_H_
#define _FIREAIR_LIB_H_

#include <stdbool.h>
#include "common_def.h"

typedef struct {
     void (*play)(const char* uri);
     void (*pause)(void);
     void (*stop)(void);
     void (*next)(void);
     void (*previous)(void);
     void (*setNextUri)(const char* uri);
     void (*seek)(int position);
     int (*getDuration)(void);
     int (*getPosition)(void);
     void (*setMute)(bool);
     bool (*getMute)(void);
     void (*setVolume)(int volume);
     int (*getVolume)(void);
     void (*setPlaymode)(int playmode);
     int (*getPlaymode)(void);
     int (*getState)(void);
     int (*rotate)(int degree);
     int (*scale)(int ratio);
     int (*setVideoSizeMode)(int mode);
	 void (*setNetwork)(const char* ssid, const char* key);
	 void (*initialize)();
     void (*fetchFmt)(int bit, int rate, int channel);
}PlayerStruct;

PlayerStruct player_airplay;
PlayerStruct player_dlna;

//#define IS_PLAY  0x10
//#define IS_PAUSE 0x20
//#define IS_STOP  0x30
//#define IS_DOWNLOAD  0x40

#define PUSH_DLNA  0x100
#define PUSH_AIRPLAY  0x200


/*****************************
DLNA
*****************************/
#define UPNP_PLAYBACK_STATE_ERROR 0
#define UPNP_PLAYBACK_STATE_STOP 1
#define UPNP_PLAYBACK_STATE_PLAY 2
#define UPNP_PLAYBACK_STATE_PAUSE 3
#define UPNP_PLAYBACK_STATE_TRANSITION 4
#define UPNP_PLAYBACK_STATE_NOMEDIA 5

//#define BUFFER_PACKET_NUM 256
//#define CACHE_PACKET_NUM 128

typedef void* UPnPManagerHandle;
typedef void* MediaRendererHandle;
typedef void* MediaServerHandle;
typedef void* MediaControllerHandle;
typedef void* MediaBrowserHandle;

UPnPManagerHandle CreateDlnaManager();
void DestroyDlnaManager(UPnPManagerHandle manager);

MediaRendererHandle CreateMediaRenderer(UPnPManagerHandle manager,  const char* name,  
					  const char* uuid,  const char* device_id, const char*  manufacture_id,  const char*  pre_shared_key);
void DestroyMediaRenderer(UPnPManagerHandle manager, MediaRendererHandle renderer);
void SetDlnaPlayer(MediaRendererHandle renderer, PlayerStruct* player);
void HWPlay(MediaRendererHandle renderer);
void HWPause(MediaRendererHandle renderer);
void HWNext(MediaRendererHandle renderer);
void HWPrevious(MediaRendererHandle renderer);
bool IsQplay(MediaRendererHandle renderer);


/*****************************
AIRPLAY
*****************************/
typedef void* AirPlayManagerHandle;

AirPlayManagerHandle CreateAirplayManager();
void DestroyAirplayManager(AirPlayManagerHandle manager);

void StartAirplay(AirPlayManagerHandle manager, const char* name);
void StopAirplay(AirPlayManagerHandle manager);
void SetAirplayPlayer(AirPlayManagerHandle manager, PlayerStruct* player);
bool IsAirplayPlaying(AirPlayManagerHandle manager);
void SuspendAirplay(AirPlayManagerHandle manager);
void SetAirplayAOInf(AirPlayManagerHandle manager, int ao_Inf);
int GetAirplayAOInf(AirPlayManagerHandle manager);

void SetAirplayPlayPause(AirPlayManagerHandle manager);// set playpause from key
void SetAirplayPlay(AirPlayManagerHandle manager);
void SetAirplayPause(AirPlayManagerHandle manager);
void SetAirplayNext(AirPlayManagerHandle manager);
void SetAirplayPrevious(AirPlayManagerHandle manager);
void AdjAirplayVol(AirPlayManagerHandle manager,int vol_adj);//set volume from key,if vol_adj>0 add volume  vol_adj<0 dec volume


typedef struct fireair_context {
    char device_name[32];
    char  uuid[9];
    const char* device_id;
    const char* manufacture_id;
    const char pre_shared_key[17];

    UPnPManagerHandle dlna_manager;
    AirPlayManagerHandle airplay_manager;
    MediaRendererHandle dlna_renderer;
    
} s_fa_context;

#endif

