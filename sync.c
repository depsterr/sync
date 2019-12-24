#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define bool unsigned char
#define false 0
#define true !false
#define PATH_MAX 256

typedef enum Mode{
	PULL = 0,
	PUSH = 1,
	CREATE = 2,
}Mode;

typedef struct Sync{
	unsigned int ngitdirs;
	char** gitdirs;
	unsigned int nmakedirs;
	char** makedirs;
	unsigned int nrcdirs;
	char** rcdirs;
}Sync;


Sync sync;

Mode mode;

bool hasFilePath;

char* currentdir;

void getOutput(char** returnstr, char* command){

	char cdcommand[PATH_MAX];

	FILE *fp;
	*returnstr = malloc(PATH_MAX);
	
	fp = popen(command, "r");
	
	ssize_t size;

	size = getdelim(returnstr, &size, 0x0, fp);
	
	/* close */
	pclose(fp);
}

void writesync(char* filepath){

	FILE* fp = fopen (filepath, "a");

	if(fp){
		for(int n = 0; n < sync.ngitdirs; n++)
			fprintf(fp, "%c%s\n", 'g', sync.gitdirs[n]);
		for(int n = 0; n < sync.nmakedirs; n++)
			fprintf(fp, "%c%s\n", 'm', sync.makedirs[n]);
		for(int n = 0; n < sync.nrcdirs; n++)
			fprintf(fp, "%c%s\n", 'r', sync.rcdirs[n]);
		fprintf(fp, "%c", 'e');
	}else{
		printf("Cannot open file for writing!\n");
		exit(-1);
	}
}

void readsync(char* filepath){

	sync.gitdirs = NULL;
	sync.makedirs = NULL;
	sync.rcdirs = NULL;

	sync.ngitdirs = 0;
	sync.nmakedirs = 0;
	sync.nrcdirs = 0;

	/* Load synfile into buffer */

	char* buffer = NULL;
	unsigned int bufferLength;
	FILE* fp = fopen (filepath, "rb");

	if (!fp) {
		printf("Syncfile does not exist!\n");
		exit(-1);
	} else {
		fseek(fp, 0, SEEK_END);
		bufferLength = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		buffer = malloc(bufferLength);
		if (buffer) {
			fread(buffer, 1, bufferLength, fp);
		}else{
			printf("Syncfile does not exist!\n");
			exit(-1);
		}
		fclose(fp);
	}

	/* Process buffer */

	typedef enum ReadMode{
		TAG = 0,
		GIT = 1,
		MAKE = 2,
		RC = 3,
		END = 4
	}ReadMode;

	ReadMode readMode = TAG;

	unsigned int filepathLength = 0;
	unsigned int startFilepath = 0;

	for(int n = 0; n < bufferLength; n++){
		if(readMode == END)
			break;
		switch(readMode){
			case GIT:
				filepathLength++;
				if(buffer[n] == '\n'){
					readMode = TAG;
					if(filepathLength > 1){
						sync.ngitdirs++;
						sync.gitdirs = realloc(sync.gitdirs, sync.ngitdirs * sizeof(char*));
						sync.gitdirs[sync.ngitdirs - 1] = malloc(filepathLength);
						for(int k = 0; k < filepathLength; k++)
							sync.gitdirs[sync.ngitdirs - 1][k] = buffer[startFilepath + k];
						sync.gitdirs[sync.ngitdirs - 1][filepathLength - 1] = '0';

					}
				}
				break;
			case MAKE:
				filepathLength++;
				if(buffer[n] == '\n'){
					readMode = TAG;
					if(filepathLength > 1){
						sync.nmakedirs++;
						sync.makedirs = realloc(sync.makedirs, sync.nmakedirs * sizeof(char*));
						sync.makedirs[sync.nmakedirs - 1] = malloc(filepathLength);
						for(int k = 0; k < filepathLength; k++)
							sync.makedirs[sync.nmakedirs - 1][k] = buffer[startFilepath + k];
						sync.makedirs[sync.nmakedirs - 1][filepathLength - 1] = '0';

					}
				}
				break;
			case RC:
				filepathLength++;
				if(buffer[n] == '\n'){
					readMode = TAG;
					if(filepathLength > 1){
						sync.nrcdirs++;
						sync.rcdirs = realloc(sync.rcdirs, sync.nrcdirs * sizeof(char*));
						sync.rcdirs[sync.nrcdirs - 1] = malloc(filepathLength);
						for(int k = 0; k < filepathLength; k++)
							sync.rcdirs[sync.nrcdirs - 1][k] = buffer[startFilepath + k];
						sync.rcdirs[sync.nrcdirs - 1][filepathLength - 1] = '0';

					}
				}
				break;
			case TAG:
				if(buffer[n] == 'g'){
					readMode = GIT;
					startFilepath = n;
					filepathLength = 0;
				}
				if(buffer[n] == 'm'){
					readMode = MAKE;
					startFilepath = n;
					filepathLength = 0;
				}
				if(buffer[n] == 'r'){
					readMode = RC;
					startFilepath = n;
					filepathLength = 0;
				}
				if(buffer[n] == 'e'){
					readMode = END;
					filepathLength = 0;
				}
				break;
		}
		
	}

	free(buffer);
}

int main(int argc, char** argv){

	/* Handle args */

	if(argc < 2){
		printf("Not enough arguments given, usage: dsync < pull/push/create > [ syncfile ]\n");
		return -1;
	}

	if(!strcmp(argv[1], "pull"))
		mode = PULL;
	else if(!strcmp(argv[1], "push"))
		mode = PUSH;
	else if(!strcmp(argv[1], "create"))
		mode = CREATE;
	

	if(argc > 2)
		hasFilePath = true;
	else
		hasFilePath = false;

   	getOutput(&currentdir, "echo $PWD");

	if(mode != CREATE){

		/* Read Syncfile */

		if(hasFilePath)
			readsync(argv[PATH_MAX]);
		else
			readsync(".syncfile");

		char cdcommand[PATH_MAX];
		if(mode == PULL){

			/* Git dirs  */

			printf("Pulling %d git repos\n\n", sync.ngitdirs);
			for(int n = 0; n < sync.ngitdirs; n++){
				printf("Running %s\n", sync.gitdirs[n]);
				strncpy(cdcommand, "cd ", PATH_MAX);
				strcat(cdcommand, sync.gitdirs[n]);
				system(cdcommand);
				system("git pull");
			}

			strncpy(cdcommand, "cd ", PATH_MAX);
			strcat(cdcommand, currentdir);
			system(cdcommand);

			/* Make dirs */

			printf("Installing %d programs with make\n\n", sync.nmakedirs);
			for(int n = 0; n < sync.nmakedirs; n++){
				printf("Running %s\n", sync.makedirs[n]);
				strncpy(cdcommand, "cd ", PATH_MAX);
				strcat(cdcommand, sync.makedirs[n]);
				system(cdcommand);
				system("sudo make install");
			}

			strncpy(cdcommand, "cd ", PATH_MAX);
			strcat(cdcommand, currentdir);
			system(cdcommand);

			/* rcdirs */
 
			printf("Running %d pullrcs\n\n", sync.nmakedirs);
			for(int n = 0; n < sync.nrcdirs; n++){
				printf("Running %s\n", sync.rcdirs[n]);
				strncpy(cdcommand, "cd ", PATH_MAX);
				strcat(cdcommand, sync.rcdirs[n]);
				system(cdcommand);
				system("sh .syncpullrc");
			}

			strncpy(cdcommand, "cd ", PATH_MAX);
			strcat(cdcommand, currentdir);
			system(cdcommand);

		} else if(mode == PUSH){

			/* rcdirs */

			printf("Running %d pushrcs\n\n", sync.nrcdirs);
			for(int n = 0; n < sync.nrcdirs; n++){
				printf("Running %s\n", sync.rcdirs[n]);
				strncpy(cdcommand, "cd ", PATH_MAX);
				strcat(cdcommand, sync.rcdirs[n]);
				system(cdcommand);
				system("sh .syncpushrc");
			}

			strncpy(cdcommand, "cd ", PATH_MAX);
			strcat(cdcommand, currentdir);
			system(cdcommand);

			/* Git dirs  */

			printf("Pushing %d git dirs\n\n", sync.ngitdirs);
			for(int n = 0; n < sync.ngitdirs; n++){
				printf("Pushing %s\n", sync.gitdirs[n]);
				strncpy(cdcommand, "cd ", PATH_MAX);
				strcat(cdcommand, sync.gitdirs[n]);
				system(cdcommand);
				system("git add --all");
				system("git commit");
				system("git push");
			}
		}
	}else{
		printf("Enter all git dirs and then enter 'end'\n\n");
		for(;;){
			char input[PATH_MAX];
			scanf("%s", &input);
			if(!strcmp(input, "end"))
				break;
			else{
				realpath(input, input);
				sync.ngitdirs++;
				sync.gitdirs = realloc(sync.gitdirs, sync.ngitdirs * sizeof(char*));
				sync.gitdirs[sync.ngitdirs - 1] = malloc(strlen(input));
				strncpy(sync.gitdirs[sync.ngitdirs - 1], input, PATH_MAX);
			}
		}
		printf("Enter all make dirs and then enter 'end'\n\n");
		for(;;){
			char input[PATH_MAX];
			scanf("%s", &input);
			if(!strcmp(input, "end"))
				break;
			else{
				realpath(input, input);
				sync.nmakedirs++;
				sync.makedirs = realloc(sync.makedirs, sync.nmakedirs * sizeof(char*));
				sync.makedirs[sync.nmakedirs - 1] = malloc(strlen(input));
				strncpy(sync.makedirs[sync.nmakedirs - 1], input, PATH_MAX);
			}
		}
		printf("Enter all rc dirs and then enter 'end'\n\n");
		for(;;){
			char cdcommand[PATH_MAX];
			char touchcommand[PATH_MAX];
			char input[PATH_MAX];
			scanf("%s", &input);
			if(!strcmp(input, "end"))
				break;
			else{
				realpath(input, input);
				sync.nrcdirs++;
				sync.rcdirs = realloc(sync.rcdirs, sync.nrcdirs * sizeof(char*));
				sync.rcdirs[sync.nrcdirs - 1] = malloc(strlen(input));
				strncpy(sync.rcdirs[sync.nrcdirs - 1], input, PATH_MAX);

				strncpy(cdcommand, "cd ", PATH_MAX);
				strcat(cdcommand, currentdir);
				system(cdcommand);

				system("touch .syncpullrc");
				system("touch .syncpushrc");
			}
		}
		if(hasFilePath)
			writesync(argv[2]);
		else
			writesync(".syncfile");
		printf("wrote .syncfile\n");
	}
}
