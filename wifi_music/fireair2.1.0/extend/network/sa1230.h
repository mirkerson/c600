#ifndef _DECODER_RS_H_
#define _DECODER_RS_H_

#define RET_DEC_ERROR -1
#define RET_DEC_NORMAL 0
#define RET_DEC_NOTREADY 1
#define RET_DEC_END 2

void* decode_rs_create(int max_strlen );
int decode_rs_getbsize(void* handle);
int decode_rs_fedpcm(void* handle, short* pcm);
int decode_rs_getstr(void* handle, unsigned char* str);
void decode_rs_destroy(void* handle);
#endif

