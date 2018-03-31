#ifndef ALI_MUSIC_H
#define ALI_MUSIC_H

#include "factory_api.h"
#include "ad_type.h"

typedef struct  
{  
    vtable *vptr;  
    void (*open_service)(void);
    void (*close_service)(void);
    void (*into_dormancy)(void);
    void (*outof_dormancy)(void);
    int  (*simple_config)(void);
    int  (*sendmsg)(char* method , char* name);
    int (*check_next)(void);
    void (*go_play_next)(void);
    int  (*analyze_pcm)(void);
    int (*notify_app)(void);


}mali;
extern vtable alimode;


#define MAC_ADRESS_FILE_NAME   "/tmp/conf/mac-address.conf"



#define FREE_MEM(ptr)        if(ptr){free((void *)ptr); ptr = NULL;}
#define FREE_JSON_OBJ(ptr)   if(ptr){json_object_put(ptr); ptr = NULL;}
#define FREE_JSON_FRAME(ptr) if(ptr){    \
        FREE_JSON_OBJ(ptr->pRootObj);    \
        FREE_MEM(ptr); ptr = NULL;}


#define REC_ALI_FILE "/tmp/ali_rec.pcm"

#define OFFLINE_MODE  0X1
#define ONLINE_MODE   0X2

typedef struct list_node{
    char *fullname;//the file path
    struct list_node *next;
    struct list_node *prev;
}node,*pnode;

typedef struct fnode
{
	struct fnode *next;
	unsigned int level;
	char *name;
	char *fullname;	
}filenode;

typedef struct head
{
	struct fnode *head;
	struct fnode *rear;
}headnode;

struct share{
	long id_tmp;
	char name_tmp[40];
};

int ali_simple_config(void);
int ali_check_next(void);
void ali_go_play_next(void);
int ali_analyze(void);
int ali_notify_app(void);



#endif

