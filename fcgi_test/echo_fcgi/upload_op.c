#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include "md5.h"
#include "upload_op.h"
#include "make_log.h"

#define TAG 		"CGI"
#define FILENAME 	"upload_op"

/**
 * @brief  将文件上传至存储服务器
 * 
 * @param conf_filename         配置文件路径 
 * @param local_filename        上传文件路径
 * @param out_id        		存储后的文件id //out
 *
 * @returns   
 *   0 成功  	非0 失败
 */
int upload(const char *conf_filename, const char *local_filename, char file_id[128])
{
	LOG(TAG, FILENAME, "---func upload begin----;");
	int result = 0;
	ConnectionInfo *pTrackerServer;
	ConnectionInfo storageServer;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	int store_path_index;

	if(conf_filename == NULL || local_filename == NULL)
	{
		printf("Usage with <config_file> <local_filename>\n");
		return 1;
	}

	result = fdfs_client_init(conf_filename);
	if(result != 0)
	{
		LOG(TAG, FILENAME, "fdfs_client_init error; result: %d", result);
		return result;
	}

	pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		LOG(TAG, FILENAME, "tracker_get_connection error;");
		return -1;
	}

	*group_name = '\0';
	store_path_index = 0;
	if ((result=tracker_query_storage_store(pTrackerServer, \
					&storageServer, group_name, &store_path_index)) != 0)
	{
		fdfs_client_destroy();
		LOG(TAG, FILENAME, "tracker_query_storage fail; result: %d", result);
		return result;
	}

	result = storage_upload_appender_by_filename1(pTrackerServer, \
			&storageServer, store_path_index, \
			local_filename, NULL, \
			NULL, 0, group_name, file_id);
	if (result != 0)
	{ 
		LOG(TAG, FILENAME, "upload file fail; result: %d", result);

		tracker_disconnect_server_ex(pTrackerServer, true);
		fdfs_client_destroy();
		return result;
	}
	//printf("%s\n", file_id);
	LOG(TAG, FILENAME, "result: %s", file_id);

	tracker_disconnect_server_ex(pTrackerServer, true);
	fdfs_client_destroy();
	
	LOG(TAG, FILENAME, "---func upload end----;");

	return result;
}

static int ReadFileInfo(const char *filename, size_t *filelen, char **tmpbuf)
{
	int ret = 0;
	char *red_buf = NULL;
	int len = 0;
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL)
	{
		LOG(TAG, FILENAME, "Open file [%s] error!", filename);
		ret = -1;
		goto END;
	}
	
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	red_buf = (char *)malloc(len + 1);
	
	rewind(fp);
	fread(red_buf, 1, len, fp);
	
	
END:
	*filelen = len;
	*tmpbuf = red_buf;
	if(fp != NULL)
		fclose(fp);
	
	return ret;
}


/**
 * @brief  对文件进行md5加密, 生成md5码
 * 
 * @param msg         			文件名称 
 * @param out_md5        		生成的md5码
 *
 * @returns   
 *   0 成功  	非0 失败
 */
int md5_encode(const char *msg, char out_md5[48]) 
{
    size_t len = 0;
    int i, ret = 0;
    uint8_t result[16];
	char *file_ptr = NULL;
 
    LOG(TAG, FILENAME, "---func upload begin----;");
	ret = ReadFileInfo(msg, &len, &file_ptr);
	if(ret != 0)
		return ret;
 
    // benchmark
    for (i = 0; i < 10; i++) {
        md5((uint8_t*)msg, len, result);
    }
 
    // get result
    for (i = 0; i < 16; i++)
        sprintf(out_md5+2*i, "%2.2x", result[i]);
    
	out_md5[32] = '\0';
	
	LOG(TAG, FILENAME, "[md5 conde:] %s", out_md5);
	
	if(file_ptr != NULL)
		free(file_ptr);
 
	LOG(TAG, FILENAME, "---func upload end----;");
	
    return ret;
}