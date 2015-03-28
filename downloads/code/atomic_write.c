/*
 * Copyright (C) 2013 Yongbing Chen <yongbing.chen.wh@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define CHECK_RESULT(x) \
	do { \
		if (x != 0){ \
			printf("error %s\n", strerror(errno)); \
			return -1; \
		} \
	}while (0);

#ifndef TEMP_FAILURE_RETRY 
#define TEMP_FAILURE_RETRY(x) \
	({ \
	 int _result; \
	 do _result = (int) (x); \
	 while (_result == -1 && errno == EINTR); \
	 _result; \
	 })
#endif


static int copy_file(const char *src_file, const char *dst_file)
{
	int ret = -1;
	FILE *fp1, *fp2;
	fp1 = fopen(src_file,"r");
	if(fp1 == NULL){
		printf("open %s error %s\n",src_file, strerror(errno));
		return ret;
	}

	fp2 = fopen(dst_file,"w");
	if(fp2 == NULL){
		printf("open %s error %s\n", dst_file, strerror(errno)); fclose(fp1);
		return ret;
	}

	fseek(fp1, 0 , SEEK_END);
	int file_size = ftell(fp1);
	fseek(fp1, 0 , SEEK_SET);
	if (file_size == 0){
		fclose(fp1); fclose(fp2);
		return 0;
	}
	char *buffer = (char*)malloc(file_size);
	if (NULL == buffer){
		printf("error %s\n", strerror(errno)); fclose(fp1); fclose(fp2);
		return -1;
	}

	printf("copy len %d from %s to %s\n", file_size, src_file, dst_file);
	ret = fread(buffer, file_size, 1, fp1);
	if (ret != 1){
		printf("ret %d, error %s\n",ret, strerror(errno));
		goto error;
	}

	ret = fwrite(buffer, file_size, 1, fp2);
	if (ret != 1){
		printf("ret %d, error %s\n",ret, strerror(errno));
		goto error;
	}
	ret = 0;

error:
	fclose(fp1);
	fclose(fp2);
	free(buffer);
	return ret;
}


static int update_config_to_tmp_file(const char* curr_file, const char* file_name)
{
	int fd = open(file_name, O_CREAT | O_APPEND | O_RDWR, 0660);
	if(fd > 0){
		if(access(curr_file,  F_OK) == 0)
			CHECK_RESULT(copy_file(curr_file, file_name));
		int test_data = rand();
		printf("writing data %d to config file %s\n", test_data, file_name);
		int data_write = TEMP_FAILURE_RETRY(write(fd, &test_data, sizeof(test_data)));
		if (data_write < 0 || data_write != sizeof(test_data)){
			printf("%s failed len:%d %s",__func__, data_write, strerror(errno));
			return -1;
		}
	}
	else 
		return -1;

	close(fd);
	return 0;

}

static int sync_conf_file(const char* file_name)
{
	int ret = -1;
	int fd = open(file_name, O_RDONLY);
	if(fd > 0){
		ret = fsync(fd);
	}
	close(fd);
	return ret;
}

int atomic_update_config_file()
{
	const char* file_name = "config";
	const char* file_name_new = "config.new";
	CHECK_RESULT(update_config_to_tmp_file(file_name, file_name_new));//open, write, close inside this operation.
	CHECK_RESULT(sync_conf_file(file_name_new));//open, fsync, close to achive a sync.
	CHECK_RESULT(rename(file_name_new, file_name));
	return 0;
}

int main(int argc, char **argv)
{
	int ret = atomic_update_config_file();
	if (ret != 0){
		printf("update config failed, keeping original one.\n");
	}
	return 0;
}
