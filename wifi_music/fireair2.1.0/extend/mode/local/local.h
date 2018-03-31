
#include "factory_api.h"

#define WORD 40

typedef struct  
{  
    vtable *vptr;  
    void (*insert)(void); 
    void (*unplug)(void); 
    int (*check_next)(void);
    void (*go_play_next)(void);
    void (*go_play_prev)(void);

}mlocal;
extern vtable localmode;

typedef struct list_node{
    char *fullname;//the file path
    struct list_node *next;
    struct list_node *prev;
}node,*pnode;

typedef struct node
{
	struct node *next;
	unsigned int level;
	char *name;
	char *fullname;	
}filenode;


typedef struct head
{
	struct node *head;
	struct node *rear;
}headnode;

struct share{
	long id_tmp;
	char name_tmp[WORD];
};

static void dir_tree(char *dirname,headnode *link_stack, int level);
static void pop_file_tree(headnode *link_stack);  

void sd_insert();
void sd_unplug();
int local_check_next(void);
void local_go_play_next(void);
void local_go_play_prev(void);
