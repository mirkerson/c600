
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "ad_type.h"
#include "local.h"



static pnode playlist_head;

static pnode playlist_tmp;
static pnode playlist_cur = NULL;
static pnode playlist_save= NULL;//local play list

//Support music format
const char *support_type[7]={
	[0] = ".mp3",
	[1] = ".wav",
	[2] = ".flac",
	[3] = ".ape",
	[4] = ".ogg",
	[5] = ".m4a",
	[6] = ".aac"
};
                                                   

/* Traverse the list,find the node to cur music  */
static pnode search_list(pnode head,char para[])
{
	pnode tmp = NULL;
	tmp = head->next;
	
	if(strcmp(head->fullname,para) == 0){
		return head;
	}
	else{	
		while(tmp != head)
		{	
			if (strcmp(tmp->fullname, para) == 0){
				return tmp; 		
			}
			else {
				tmp= tmp->next;
			}
		}
	}
	return NULL;
}

/* print all node to the playlist */
static void print_list(pnode head)
{
	pnode tmp = head->next;
	printf("\nThe song of Current directory:\n\n");

	if(head->fullname!=NULL){
		printf("head:%s\n",head->fullname);
		while(tmp != head){
			printf("node:%s\n",tmp->fullname);
			tmp = tmp->next;
		}
	}else{
		printf("\nfound nothing!\n");
	}
}

/* del playlst*/
static void clean_list(pnode head)
{
	if(NULL == head)
		return;
		
	pnode tmp = head->next;
	printf("\nclean Current playlist:\n\n");
	while(tmp != head){
		//printf("delete node:%s\n",tmp->fullname);
		free(tmp->fullname);
		tmp = tmp->next;
		free(tmp->prev);
	}
	//printf("delete head:%s\n",head->fullname);
	free(head->fullname);
	free(head);
}


//~{;qH!~}dir~{ND<~<POB5DKySPND<~#(0|@(D?B<#)#,RT6SAPPNJ=75;X~}
static headnode *read_dir_to_link_stack(char *dir,int level)
{
	DIR *dirp = NULL;
	
	if(NULL== (dirp=opendir(dir)))
	{
		perror("opendir");
		exit(EXIT_FAILURE);
	}	

	headnode *ptr = (headnode *)malloc (sizeof (headnode));
	if(ptr==NULL)
	{
		perror("malloc");exit(EXIT_FAILURE);
	}
	ptr->head = NULL;
	ptr->rear = NULL;	
	
	struct dirent *entp = NULL;
	while ( NULL != (entp =readdir(dirp)))
	{
		if (strcmp(entp->d_name, "..")==0 || strcmp(entp->d_name, ".")==0)	// ./ ../
		{
			continue;
		}
		else
		{
			filenode *temp = (filenode *)malloc(sizeof(filenode));
			if(temp==NULL)
			{
				perror("malloc");exit(EXIT_FAILURE);
			}
			temp->next = NULL;
			temp->level = level;
			
			temp->name = (char *)malloc(strlen( entp->d_name) + 1);
			sprintf(temp->name ,"%s\0", entp->d_name);
			
			temp->fullname = (char *)malloc(strlen(dir)+1+strlen( entp->d_name) + 1);
			sprintf(temp->fullname,"%s/%s\0",dir,entp->d_name);
			if(ptr->head == NULL)
			{
				ptr->head = temp;
				ptr->rear = temp;		
			}	
			else		
			{
				temp->next = ptr->head;
				ptr->head = temp;
			}
		}		
	}
	closedir(dirp);
	return ptr;	
}
/*
	type:1, regufile. 2 dir.
*/
static void out_file_info(filenode *ptr)
{
	int i;
	printf("|");
	for(i = 0;i < ptr->level; i++)
	{
		printf("    ");
	}
	printf("|-- ");
	printf("%s\n",ptr->name);
}

static void insert_to_playlist(filenode *ptr)	
{
	pnode new = NULL;            
	if(playlist_head->fullname == NULL)	
	{
		playlist_head->fullname = (char *)malloc(strlen( ptr->fullname)+1);
		strcpy(playlist_head->fullname,ptr->fullname);
	}	
	else
	{
		new = (pnode)malloc(sizeof(node));
		if(new == NULL){
			perror("malloc");
			exit(EXIT_FAILURE); 
		}
		new->fullname = (char *)malloc(strlen( ptr->fullname)+1);
		if(new->fullname == NULL){
			perror("malloc");
			exit(EXIT_FAILURE); 
		}
		strcpy(new->fullname,ptr->fullname);	
		playlist_tmp->next = new;
		new->prev = playlist_tmp;
		new->next = NULL;
		playlist_tmp = new;
	}
}

static void dir_tree(char *dirname,headnode *link_stack, int level)
{
	headnode *ret = NULL;
	//~{6AH!D?B<PEO"#,75;XPB5D6SAP~}
	ret = read_dir_to_link_stack(dirname, level+1);
	//~{=+PB6SAP7E5=T-6SAPG0~}
	if(link_stack->head != NULL && ret->head != NULL)
	{
		ret->rear->next = link_stack->head;
		link_stack->head = ret->head;
	}
	
	if(link_stack->head == NULL && ret->head != NULL )
		link_stack->head = ret->head;
	
	if(link_stack->rear == NULL && ret->rear != NULL)
		link_stack->rear = ret->rear;

	free(ret);

	pop_file_tree(link_stack);
}

static void pop_file_tree(headnode *link_stack)
{
	int i;
	while(link_stack->head != NULL )	
	{
		struct stat stat_src;
		if (lstat(link_stack->head->fullname, &stat_src) != 0) 
		{
			fprintf(stderr, "%s(%d): stat error(%s)!\n", __FILE__, __LINE__, strerror(errno));			
		}	
	
		if(S_ISDIR(stat_src.st_mode))	
		{
			filenode *temp = link_stack->head;
			link_stack->head = link_stack->head->next;	
			if(link_stack->head == NULL)
				link_stack->rear =NULL;	
			//out_file_info(temp);
			
			dir_tree(temp->fullname,link_stack,temp->level);	
			
			free(temp->name);
			free(temp->fullname);
			free(temp);
		}
		else
		{
			filenode *temp = link_stack->head;
			link_stack->head = link_stack->head->next;	
			if(link_stack->head == NULL)
				link_stack->rear =NULL;
			
			//out_file_info(temp);
			
			for(i=0;i<sizeof(support_type)/sizeof(char *);i++)	
			{
				if(strstr(temp->name,support_type[i]) != NULL)
				{
					insert_to_playlist(temp);	
					break;
				}
			}
			
			free(temp->name);
			free(temp->fullname);
			free(temp);
		}
	}
}

static void search_music(char *mediadir)
{

	playlist_head=NULL;

	playlist_head = (pnode)malloc(sizeof(node));                                         
	if(playlist_head == NULL){
		perror("malloc");exit(EXIT_FAILURE);                                                                
	}                                                                             	
	playlist_head->next = NULL;                                                            
    playlist_head->prev = NULL;                                                           
	playlist_head->fullname=NULL;
	playlist_tmp=playlist_head;

	headnode *link_stack = (headnode *)malloc (sizeof (headnode));
	if(link_stack == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	link_stack->head = NULL;	/*~{6SJW~}*/
	link_stack->rear = NULL;	/*~{6SN2~}*/
		
	printf("%s\n",mediadir);
	dir_tree(mediadir, link_stack,-1);
	
	//~{Q-;7~}
	playlist_head->prev = playlist_tmp;
	playlist_tmp->next = playlist_head;
	
	free(link_stack);	
}

void sd_insert()
{
	search_music("/mnt/sdcard");		//Find out music files and Create a doubly linked list
	print_list(playlist_head);		//display playlist
	playlist_save=playlist_head;
}

void sd_unplug()
{
	playlist_cur = NULL;
	clean_list(playlist_head);		//Clear playlist
	playlist_save = NULL;
}


int mlocal_init(void)
{
    int ret;

    
    ret = adplayer_init();
    if(OK != ret)
        return ret;    
}

void mlocal_exit(void)
{
    adplayer_exit();
}


int mlocal_play(char *uri)
{
    if(NULL == playlist_save)
        return ERROR;

    return ffc_inst.player.adplayer(AD_PLAY, playlist_save->fullname);
}

void mlocal_pause(void)
{
#if 0
ffc_inst.player.tone_play("/home/ad/aplianwang.mp3", 0);

#else
    ffc_inst.player.adplayer(AD_PAUSE, NULL);
#endif
}

void mlocal_stop(void)
{
    ffc_inst.player.adplayer(AD_STOP, NULL);
}

void mlocal_get_file_attr(s_file_context *fc)
{
    ffc_inst.player.get_file_context(fc);
}

void mlocal_set_file_attr(s_file_context *fc)
{
    ffc_inst.player.set_file_context(fc);
}

int mlocal_get_status()
{
    return ffc_inst.player.adplayer(AD_GET_STATUS, NULL);
}


vtable localmode = { 
        &mlocal_init,
        &mlocal_exit,
        &mlocal_play, 
        NULL, 
        &mlocal_pause, 
        &mlocal_stop,
        NULL,
        &mlocal_get_file_attr,
        &mlocal_set_file_attr,
        &mlocal_get_status
    }; 

int local_check_next(void)
{    
    return ffc_inst.player.adplayer(AD_READY_GO_NEXT, NULL);
}

void local_go_play_next(void)
{

    if(NULL == playlist_save)
        return;

    LOG_PRT();

    playlist_save = playlist_save->next;
    ffc_inst.player.adplayer(AD_STOP, NULL);
    ffc_inst.player.adplayer(AD_PLAY, playlist_save->fullname);

}

void local_go_play_prev(void)
{
    if(NULL == playlist_save)
        return;

    LOG_PRT();

    playlist_save = playlist_save->prev;
    ffc_inst.player.adplayer(AD_STOP, NULL);
    ffc_inst.player.adplayer(AD_PLAY, playlist_save->fullname);
}

