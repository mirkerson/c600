#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>

#define MAX_SCRIPT_BUF_SIZE (64*1024)

typedef struct
{
        int  main_key_count;
        int  version[3];
} script_head_t;

typedef struct
{
        char main_name[32];
        int  lenth;
        int  offset;
} script_main_key_t;

typedef struct
{
        char sub_name[32];
        int  offset;
        int  pattern;
} script_sub_key_t;

typedef enum
{
        SCIRPT_PARSER_VALUE_TYPE_INVALID = 0,
        SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD,
        SCIRPT_PARSER_VALUE_TYPE_STRING,
        SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD,
        SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD
} script_parser_value_type_t;


script_sub_key_t *get_sub_key(const char *script_buf, const char *main_key, const char *sub_key)
{
	script_head_t *hd = NULL;
	script_main_key_t *mk = NULL;
	script_sub_key_t *sk = NULL;
	int i, j, p;

	if (main_key == NULL || sub_key == NULL) {
		return NULL;
	}

	hd = (script_head_t *)script_buf;
	mk = (script_main_key_t *)(hd + 1);

	for (i = 0; i < hd->main_key_count; i++) {
		if (strcmp(main_key, mk->main_name)) {
			mk++;
			continue;
		}

		for (j = 0; j < mk->lenth; j++) {
			sk = (script_sub_key_t *)(script_buf + (mk->offset<<2) + j * sizeof(script_sub_key_t));
			if (!strcmp(sub_key, sk->sub_name)) {
				return sk;
			}
		}
	}
	return NULL;
}

/**
 * @func: script_get_int
 * @desc: get key value
 * @ret: -1 ERROR, >=0 OK
 * @arg1
 * @arg2:
 * @arg3:
 **/
int script_get_int(const char *script_buf, const char *main_key, const char *sub_key)
{
	script_sub_key_t *sk = NULL;
	char *pdata;
	int value;

	sk = get_sub_key(script_buf, main_key, sub_key);
	if (sk == NULL) {
		return -1;
	}

	if (((sk->pattern >> 16) & 0xffff) == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD) {
		pdata = (char *)(script_buf + (sk->offset<<2));
		value = *((int *)pdata);
		return value;
	}

	return -1;
}

/**
 * @func: script_set_int
 * @desc: set key value
 * @ret: -1 ERROR, 0 OK
 * @arg1
 * @arg2:
 * @arg3:
 **/
int script_set_int(const char *script_buf, const char *main_key, const char *sub_key, int value)
{
	script_sub_key_t *sk = NULL;
	char *pdata;

	sk = get_sub_key(script_buf, main_key, sub_key);
	if (sk == NULL) {
		return -1;
	}

	if (((sk->pattern >> 16) & 0xffff) == SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD) {
		pdata = (char *)(script_buf + (sk->offset<<2));

		printf("SET: %p, old=%d, %d\n", pdata, *((int *)pdata), value);

		*((int *)pdata) = value;
		return 0;
	}

	return -1;
}

/**
 * @func: script_get_str
 * @desc: get key value
 * @ret: NULL on ERROR
 * @arg1
 * @arg2:
 * @arg3:
 * @arg4: user require to clean the buf
 **/
char *script_get_str(const char *script_buf, const char *main_key, const char *sub_key, char *buf)
{
	script_sub_key_t *sk = NULL;
	char *pdata;

	sk = get_sub_key(script_buf, main_key, sub_key);
	if (sk == NULL) {
		return NULL;
	}

	if (((sk->pattern >> 16) & 0xffff) == SCIRPT_PARSER_VALUE_TYPE_STRING) {
		pdata = (char *)(script_buf + (sk->offset<<2));
		memcpy(buf, pdata, ((sk->pattern >> 0) & 0xffff));
		return (char *)buf;
	}

	return NULL;
}

void script_list(const char *script_buf)
{
	script_head_t *hd = NULL;
	script_main_key_t *mk = NULL;
	script_sub_key_t *sk = NULL;
	int i, j, p, wc;
	char *pdata;
	char buf[32];

	hd = (script_head_t *)script_buf;
	printf("main_key_count: %d, version: %d.%d.%d\n",
	       hd->main_key_count,
	       hd->version[0], hd->version[1], hd->version[2]);

	mk = (script_main_key_t *)(hd + 1);

	for (i=0; i<hd->main_key_count; i++) {
		printf("[%s]\n",  mk->main_name);

		for (j=0; j<mk->lenth; j++) {
			sk = (script_sub_key_t *)(script_buf + (mk->offset<<2) + j * sizeof(script_sub_key_t));
			p = (sk->pattern >> 16) & 0xffff;
			wc = (sk->pattern >> 0) & 0xffff;
			pdata = (char *)(script_buf + (sk->offset<<2));

			switch (p) {
			case SCIRPT_PARSER_VALUE_TYPE_INVALID:
				printf("  %s: INVALID\n", sk->sub_name);
				break;
			case SCIRPT_PARSER_VALUE_TYPE_SINGLE_WORD:
				printf("  %s: 0x%08x\n", sk->sub_name, *((int *)pdata));
				break;
			case SCIRPT_PARSER_VALUE_TYPE_STRING:
				memset(buf, 0, sizeof(buf));
				memcpy(buf, pdata, wc*4);
				printf("  %s: %s\n", sk->sub_name, buf);
				break;
			case SCIRPT_PARSER_VALUE_TYPE_MULTI_WORD:
				break;
			case SCIRPT_PARSER_VALUE_TYPE_GPIO_WORD:
				printf("  %s: GPIO\n", sk->sub_name);
				break;
			}
		}
		mk++;
	}
}

void show_help(const char *program)
{
	printf("Valid Options: [-L][-G][-S][-h][-m main_key][-s sub_key][-f script_file] [-v value]\ne.g.\n");
	printf("  %s -L -f /boot/script.bin  -- to list all config\n", program);
	printf("  %s -G -f /boot/script.bin -m pmu_para -s pmu_used  -- to get pmu_used value\n", program);
	printf("  %s -S -f /boot/script.bin -m pmu_para -s pmu_used -v 0 -- to set new value 0. only support integer type\n\n", program);
}

typedef enum {
	SCRIPT_NOP = 0,
	SCRIPT_LIST,
	SCRIPT_GET,
	SCRIPT_SET,
} script_cmd_t;

int main(int argc, char *argv[])
{
	int opt, fd, ret, size, new_value;
	script_cmd_t cmd = SCRIPT_NOP;
	char *main_key_name = NULL, *sub_key_name = NULL;
	char *buf, *script_file = NULL;

	if (argc <= 1) {
		show_help(argv[0]);
		return -1;
	}

	while ((opt = getopt(argc, argv, "LGShm:s:f:v:")) != -1) {
		switch (opt) {
		case 'h':
			show_help(argv[0]);
			return 0;
		case 'L':
			cmd = SCRIPT_LIST;
			break;
		case 'G':
			cmd = SCRIPT_GET;
			break;
		case 'S':
			cmd = SCRIPT_SET;
			break;
		case 'm':
			main_key_name = optarg;
			break;
		case 's':
			sub_key_name = optarg;
			break;
		case 'f':
			script_file = optarg;
			break;
		case 'v':
			new_value = atoi(optarg);
			break;
		default:
			show_help(argv[0]);
			return -1;
		}
	}

	if (access(script_file, W_OK|R_OK)) {
		perror("Access script file");
		return -1;
	}

	buf = malloc(MAX_SCRIPT_BUF_SIZE);
	if (buf == NULL) {
		perror("malloc");
	}

	memset(buf, 0, MAX_SCRIPT_BUF_SIZE);

	fd = open(script_file, O_RDWR);
	if (fd == -1) {
		perror("open");
		return -1;
	}

	size = read(fd, buf, MAX_SCRIPT_BUF_SIZE);
	printf("size=%d\n", size);

	switch (cmd) {
	case SCRIPT_LIST:
		script_list(buf);
		break;
	case SCRIPT_GET:
		printf("%d\n", script_get_int(buf, main_key_name, sub_key_name));
		break;
	case SCRIPT_SET:
	{
		printf("set %d, ret=%d\n", new_value, script_set_int(buf, main_key_name, sub_key_name, new_value));
		lseek(fd, 0, SEEK_SET);
		write(fd, buf, size);
		perror("write");
		fsync(fd);
		break;
	}
	default:
		show_help(argv[0]);
		break;
	}

	close(fd);
	free(buf);
	return 0;
}

