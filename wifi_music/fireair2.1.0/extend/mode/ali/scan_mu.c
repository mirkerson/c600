
#ifdef __ALI_MUSIC__

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "ali_music.h"


static void pop_file_tree(headnode *link_stack);
static void dir_tree(char *dirname,headnode *link_stack, int level);

static int ali_mode = ONLINE_MODE;  //online or offline

static pnode offlist_head;

static pnode offlist_tmp;
static pnode offlist_cur = NULL;
pnode offlist_save= NULL;

const char *cache_type=".mu";

                               
/* Traverse the list,find the node to cur music  */
static pnode search_list(pnode head,char para[])
{
	pnode tmp = NULL;
	tmp = head->next;
	
	if(strcmp(head->fullname,para) == 0)
	{
		return head;
	}
	else
	{	
		while(tmp != head)
		{	
			if (strcmp(tmp->fullname, para) == 0)
			{
				return tmp; 		
			}
			else 
			{
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

	if(head->fullname!=NULL)
	{
		printf("head:%s\n",head->fullname);
		while(tmp != head)
		{
			printf("node:%s\n",tmp->fullname);
			tmp = tmp->next;
		}
	}
	else
	{
		printf("\nfound nothing!\n");
	}
}

/* del playlst*/
static void clean_list(pnode head)
{
	pnode tmp = head->next;
	printf("\nclean Current playlist:\n\n");
	while(tmp != head)
	{
		free(tmp->fullname);
		tmp = tmp->next;
		free(tmp->prev);
	}
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
        return NULL;
	}	

	headnode *ptr = (headnode *)malloc (sizeof (headnode));
	if(ptr==NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
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
				perror("malloc");
				exit(EXIT_FAILURE);
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
	if(offlist_head->fullname == NULL)	
	{
		offlist_head->fullname = (char *)malloc(strlen( ptr->fullname)+1);
		strcpy(offlist_head->fullname,ptr->fullname);
	}	
	else
	{
		new = (pnode)malloc(sizeof(node));
		if(new == NULL)
		{
			perror("malloc");
			exit(EXIT_FAILURE); 
		}
		new->fullname = (char *)malloc(strlen( ptr->fullname)+1);
		if(new->fullname == NULL)
		{
			perror("malloc");
			exit(EXIT_FAILURE); 
		}
		strcpy(new->fullname,ptr->fullname);	
		offlist_tmp->next = new;
		new->prev = offlist_tmp;
		new->next = NULL;
		offlist_tmp = new;
	}
}

static void dir_tree(char *dirname,headnode *link_stack, int level)
{
	headnode *ret = NULL;
    
	ret = read_dir_to_link_stack(dirname, level+1);
    if(NULL != ret)
    {
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

}

static void pop_file_tree(headnode *link_stack)
{
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
            
			out_file_info(temp);
			
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
			
			out_file_info(temp);
			
			if(strstr(temp->name,cache_type) != NULL)
			{
				insert_to_playlist(temp);
			}
			
			
			free(temp->name);
			free(temp->fullname);
			free(temp);
		}
	}
}


static void search_music(char *mediadir)
{

	offlist_head=NULL;

	offlist_head = (pnode)malloc(sizeof(node));                                         
	if(offlist_head == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);                                                                
	}                                                                             	
	offlist_head->next = NULL;                                                            
    offlist_head->prev = NULL;                                                           
	offlist_head->fullname=NULL;
	offlist_tmp=offlist_head;

	headnode *link_stack = (headnode *)malloc (sizeof (headnode));
	if(link_stack == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	link_stack->head = NULL;	/*~{6SJW~}*/
	link_stack->rear = NULL;	/*~{6SN2~}*/
		
	printf("%s\n",mediadir);
	dir_tree(mediadir, link_stack,-1); //search  -r
	
	//~{Q-;7~}
	offlist_head->prev = offlist_tmp;
	offlist_tmp->next = offlist_head;
	
	free(link_stack);	
}

int ali_into_offline_mode()
{
    if(OFFLINE_MODE == ali_mode)
        return 1;
    
    ali_mode = OFFLINE_MODE;
    
	search_music("/mnt/sdcard/CUCKOO");		//Find out music files and Create a doubly linked list
	print_list(offlist_head);		//display playlist
	offlist_save=offlist_head;

    return 0;
}

int ali_outof_offline_mode()
{
    if(ONLINE_MODE == ali_mode)
        return 1;

    ali_mode = ONLINE_MODE;
    
	offlist_cur = NULL;
	clean_list(offlist_head);		//Clear playlist
	offlist_save = NULL;

    return 0;
}

int get_alistate()
{
    return ali_mode;
}

#endif

