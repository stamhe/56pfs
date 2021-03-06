/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/
#include "pfs_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

typedef struct {
	char *file;
	char *ip;
	int port;
} t_arg_upload;

void do_loop_dir(char *indir, char *ip, int port)
{
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(indir)) == NULL) 
	{
		fprintf(stderr, "opendir [%s] err %m\n", indir);
		return ;
	}
	while((dirp = readdir(dp)) != NULL) 
	{
		if (dirp->d_name[0] == '.')
			continue;
		char file[256] = {0x0};
		snprintf(file, sizeof(file), "%s/%s", indir, dirp->d_name);
		struct stat filestat;
		if(stat(file, &filestat) < 0) 
		{
			fprintf(stderr, "stat error,filename:%s\n", file);
			continue;
		}
		if (S_ISDIR(filestat.st_mode))
			do_loop_dir(file, ip, port);
		else if (S_ISREG(filestat.st_mode))
		{
			if (filestat.st_size == 0)
				continue;
			t_api_info api;
			memset(&api, 0, sizeof(api));
			api.type = TASK_DELFILE;
			snprintf(api.remotefile, sizeof(api.remotefile), "%s", file);
			snprintf(api.group, sizeof(api.group), "pic");
			int ret = operate_pfs(ip, port, &api);
			if (ret != PFS_OK)
				fprintf(stderr, "delete ERR %s:%d:%s\n", file, ret, api.errmsg);
		}
		else
			fprintf(stderr, "no process [%s]\n", file);
	}
	closedir(dp);
}

void delete_thread(t_arg_upload *u)
{
	init_ns_cache();
	do_loop_dir(u->file, u->ip, u->port);
}

int main(int c, char **v)
{
	if (c != 4)
	{
		fprintf(stderr, "Usage %s nameip nameport dir!\n", v[0]);
		return -1;
	}
	t_arg_upload u;
	memset(&u, 0, sizeof(u));
	u.file = v[3];
	u.ip = v[1];
	u.port = atoi(v[2]);

	delete_thread(&u);

	return 0;
}

