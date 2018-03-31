/*
 * utils.h
 *
 *  Created on: 2016年1月11日
 *      Author: Administrator
 */

#ifndef BBC_SDK_INCLUDES_UTILS_H_
#define BBC_SDK_INCLUDES_UTILS_H_

#define url_free_part(P) if(P) { free(P); }

typedef struct Url {
    char *scheme;
    char *hostname;
    char *port;
    char *path;
} Url;

Url *url_parse(char *url);
void url_free(Url *url);

char* get_name_from_url(char* url);
char* copy_str(char* src);
void to_hex_str(unsigned char* src, char* dest, int len);

#endif /* BBC_SDK_INCLUDES_UTILS_H_ */
