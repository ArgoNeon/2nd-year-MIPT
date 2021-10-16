#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

void nameless_wc(int argn, char* argv[], int l, int w, int c, int total) {
pid_t pid; 
int x = 1, status, fd[2], fderr[2], character = 0, word = 0, line = 0, old = 1, err = 0;
char buf;
char buferr[100];

	pipe(fd);
	pipe(fderr);
	pid = fork();

	if (pid == 0) {
		dup2 (fderr[1], 2);
		dup2 (fd[1], 1);
		close(fd[0]);
		close(fderr[0]);
		close(fd[1]);
		close(fderr[1]);
		execv("mycat", argv);	

	}else {
	close(fd[1]);
	close(fderr[1]);
	while (x > 0) {
		x = read(fd[0], &buf, 1);		
		character++;
		if ((buf == '\n') && (x > 0))
			line++;
		if ((isspace(buf) > 0) && (x > 0) && (old == 0)) {
			word++;
			old++;
		} else {
			if ((isspace(buf) == 0) && (x > 0) && (old > 0))
				old--;
		}
		if ((isspace(buf) > 0) && (x == 0)) {
			word--;
		}
	}
	character--;	
	if (character > 0)
		word++;
	
	if (argn == 1) {
		err = read(fderr[0], &buf, 1);	
	}
	
	if((argn == 1) && (err > 0)) {
		buferr[0] = buf;
		x = read(fderr[0], &buf, 1);
		while(x > 0) {
			buferr[err] = buf;
			err++;
			x = read(fderr[0], &buf, 1);
		}
		buferr[err] = '\0';
		printf("%s", buferr);
	}

	wait(&status);
	if (WIFEXITED(status) && (err == 0)) {
		if (l > 0) 
		printf("  %d", line);
		if (w > 0)
		printf("  %d", word);
		if (c > 0)
		printf("  %d", character);
		if ((l == 0) && (w == 0) && (c == 0))
		printf("      %d       %d       %d", line, word, character);
		
		switch(total) {
		case 0:
		printf("  %s\n", argv[1]);
		break;
		case 1:
		printf("  total\n");
		break;
		case 2:
		printf("\n");
		break;
		}		
	}
	}
} 
	
//------------------------------------------------------------------------------------------
int main(int argn, char* argv[]) {
	int i, get, l = 0, w = 0, c = 0;
	const char* optstring = "l:w:c:";
	char* args[3];
	
	args[0] = argv[0];
	args[2] = NULL;

	if (argn != 1) {
	while((get = getopt(argn, argv, optstring)) != -1) {
		switch(get) {
		case 'l':
			l++;
			break;
		case 'w':
			w++;
			break;
		case 'c':
			c++;
			break;
		}
	}
	
	for (i = 1; i < argn; i++) {
		args[1] = argv[i];
		nameless_wc(1, args, l, w, c, 0);
	}
	nameless_wc(argn, argv, l, w, c, 1);
	} else {
		nameless_wc(argn, argv, l, w, c, 2);
	}

	return 0;
}
