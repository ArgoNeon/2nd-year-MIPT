#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

int main(int argn, char* args[]) {

	int uid, gid, ngroups = 100, i;
	uid_t pw_uid;
	gid_t pw_gid;
	const char* username;
	const char* grname;
	struct passwd* user;
	struct group* group;
	gid_t* groups = NULL;

		if (argn == 1) {	
		pw_uid = getuid();
		pw_gid = getgid();
		uid = (int)pw_uid;
		gid = (int)pw_gid;
		ngroups = getgroups(0, NULL);
		groups = (gid_t*)calloc(ngroups, sizeof(gid_t));
		user = getpwuid(pw_uid);
		group = getgrgid(pw_gid);
		username = (const char*)user->pw_name;
		grname = (const char*)group->gr_name;

		ngroups = getgrouplist (username, pw_gid, groups, &ngroups);
	
		printf("uid=%d(%s) gid=%d(%s) groups=", uid, username, gid, grname);
	
		for (i = 0; i < ngroups; i++) {
			group = getgrgid(groups[i]);
			grname = (const char*)group->gr_name;
			printf ("%d(%s)", groups[i], grname);
			if (i < ngroups - 1)
				printf(",");
		}
		printf("\n");
		free(groups);
	}else{
		if (argn == 2) {
			username = (const char*)args[1];
			user = getpwnam(username);
			if (user == NULL) {
				printf("id: ‘%s’: no such user\n", username);	
			}else{
				group = getgrnam(username);
				pw_uid = user->pw_uid;
				pw_gid = group->gr_gid;
				uid = (int)pw_uid;
				gid = (int)pw_gid;
				grname = (const char*)group->gr_name;
				groups = (gid_t*)calloc(ngroups, sizeof(gid_t));
				ngroups = getgrouplist (username, pw_gid, groups, &ngroups);
	
				printf("uid=%d(%s) gid=%d(%s) groups=", uid, username, gid, grname);
				
				for (i = 0; i < ngroups; i++) {
					group = getgrgid(groups[i]);
					grname = (const char*)group->gr_name;
					printf ("%d(%s)", groups[i], grname);
					if (i < ngroups - 1)
						printf(",");
				}
				printf ("\n");
				free(groups);
			}
		}else{
			printf("id: extra operand ‘%s'\nTry 'id --help' for more information.\n",args[2]);
		}
	}
	return 0;
}
