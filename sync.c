#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define bool unsigned char
#define false 0
#define true !false

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


Sync syncData;

Mode mode;

bool hasFilePath;

char* currentdir;

void getOutput(char** returnstr, char* command){

	FILE *fp;
	*returnstr = malloc(256);
	
	fp = popen(command, "r");
	
	ssize_t size;

	size = getdelim(returnstr, &size, 0x0, fp);
	
	/* close */
	pclose(fp);
}

void writesyncData(char* filepath){

	FILE* fp = fopen (filepath, "w");

	if(fp){
		for(int n = 0; n < syncData.ngitdirs; n++)
			fprintf(fp, "%c%s\n", 'g', syncData.gitdirs[n]);
		for(int n = 0; n < syncData.nmakedirs; n++)
			fprintf(fp, "%c%s\n", 'm', syncData.makedirs[n]);
		for(int n = 0; n < syncData.nrcdirs; n++)
			fprintf(fp, "%c%s\n", 'r', syncData.rcdirs[n]);
		fprintf(fp, "%c", 'e');
	}else{
		printf("Cannot open file for writing!\n");
		exit(-1);
	}
}

void readsyncData(char* filepath){

	syncData.gitdirs = NULL;
	syncData.makedirs = NULL;
	syncData.rcdirs = NULL;

	syncData.ngitdirs = 0;
	syncData.nmakedirs = 0;
	syncData.nrcdirs = 0;

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
			printf("Syncfile is empty!\n");
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
						syncData.ngitdirs++;
						syncData.gitdirs = realloc(syncData.gitdirs, syncData.ngitdirs * sizeof(char*));
						syncData.gitdirs[syncData.ngitdirs - 1] = malloc(filepathLength);
						for(int k = 0; k < filepathLength; k++)
							syncData.gitdirs[syncData.ngitdirs - 1][k] = buffer[startFilepath + k];
						syncData.gitdirs[syncData.ngitdirs - 1][filepathLength - 1] = '\0';
					}
				}
				break;
			case MAKE:
				filepathLength++;
				if(buffer[n] == '\n'){
					readMode = TAG;
					if(filepathLength > 1){
						syncData.nmakedirs++;
						syncData.makedirs = realloc(syncData.makedirs, syncData.nmakedirs * sizeof(char*));
						syncData.makedirs[syncData.nmakedirs - 1] = malloc(filepathLength);
						for(int k = 0; k < filepathLength; k++)
							syncData.makedirs[syncData.nmakedirs - 1][k] = buffer[startFilepath + k];
						syncData.makedirs[syncData.nmakedirs - 1][filepathLength - 1] = '\0';
					}
				}
				break;
			case RC:
				filepathLength++;
				if(buffer[n] == '\n'){
					readMode = TAG;
					if(filepathLength > 1){
						syncData.nrcdirs++;
						syncData.rcdirs = realloc(syncData.rcdirs, syncData.nrcdirs * sizeof(char*));
						syncData.rcdirs[syncData.nrcdirs - 1] = malloc(filepathLength);
						for(int k = 0; k < filepathLength; k++)
							syncData.rcdirs[syncData.nrcdirs - 1][k] = buffer[startFilepath + k];
						syncData.rcdirs[syncData.nrcdirs - 1][filepathLength - 1] = '\0';
					}
				}
				break;
			case TAG:
				if(buffer[n] == 'g'){
					readMode = GIT;
					startFilepath = n + 1;
					filepathLength = 0;
				}
				if(buffer[n] == 'm'){
					readMode = MAKE;
					startFilepath = n + 1;
					filepathLength = 0;
				}
				if(buffer[n] == 'r'){
					readMode = RC;
					startFilepath = n + 1;
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
		printf("Not enough arguments given, usage: dsyncData < pull/push/create > [ syncDatafile ]\n");
		return -1;
	}

	if(!strcmp(argv[1], "pull"))
		mode = PULL;
	else if(!strcmp(argv[1], "push"))
		mode = PUSH;
	else if(!strcmp(argv[1], "create"))
		mode = CREATE;
	else{
		printf("Invalid argument: '%s'\n", argv[1]);
		exit(-1);
	}

	if(argc > 2)
		hasFilePath = true;
	else
		hasFilePath = false;

   	getOutput(&currentdir, "echo $PWD");

	if(mode != CREATE){

		/* Read Syncfile */

		if(hasFilePath)
			readsyncData(argv[2]);
		else
			readsyncData(".syncDatafile");

		if(mode == PULL){

			/* Git dirs  */

			printf("Pulling %d git repos\n\n", syncData.ngitdirs);
			for(int n = 0; n < syncData.ngitdirs; n++){
				printf("Running %s\n", syncData.gitdirs[n]);
				chdir(syncData.gitdirs[n]);
				system("git pull");
			}

			/* Make dirs */

			printf("Installing %d programs with make\n\n", syncData.nmakedirs);
			for(int n = 0; n < syncData.nmakedirs; n++){
				printf("Running %s\n", syncData.makedirs[n]);
				chdir(syncData.makedirs[n]);
				system("sudo make install");
			}

			/* rcdirs */
 
			printf("Running %d pullrcs\n\n", syncData.nmakedirs);
			for(int n = 0; n < syncData.nrcdirs; n++){
				printf("Running %s\n", syncData.rcdirs[n]);
				chdir(syncData.rcdirs[n]);
				system("sh .syncDatapullrc");
			}

		} else if(mode == PUSH){

			/* rcdirs */

			printf("Running %d pushrcs\n\n", syncData.nrcdirs);
			for(int n = 0; n < syncData.nrcdirs; n++){
				printf("Running %s\n", syncData.rcdirs[n]);
				chdir(syncData.rcdirs[n]);
				system("sh .syncDatapushrc");
			}

			/* Git dirs  */

			printf("Pushing %d git dirs\n\n", syncData.ngitdirs);
			for(int n = 0; n < syncData.ngitdirs; n++){
				printf("Pushing %s\n", syncData.gitdirs[n]);
				chdir(syncData.gitdirs[n]);
				system("git add --all");
				system("git commit");
				system("git push");
			}
		}
	}else{
		syncData.gitdirs = NULL;
		syncData.makedirs = NULL;
		syncData.rcdirs = NULL;

		syncData.ngitdirs = 0;
		syncData.nmakedirs = 0;
		syncData.nrcdirs = 0;

		printf("\nEnter all git dirs and then enter 'end'\n\n");
		for(;;){
			char input[256];
			char filePath[256];
			scanf("%s", &input);
			if(!strcmp(input, "end"))
				break;
			else{
				realpath(input, filePath);
				printf("%s\n", filePath);
				syncData.ngitdirs++;
				syncData.gitdirs = realloc(syncData.gitdirs, syncData.ngitdirs * sizeof(char*));
				syncData.gitdirs[syncData.ngitdirs - 1] = malloc(strlen(filePath));
				strcpy(syncData.gitdirs[syncData.ngitdirs - 1], filePath);
			}
		}
		printf("\nEnter all make dirs and then enter 'end'\n\n");
		for(;;){
			char input[256];
			char filePath[256];
			scanf("%s", &input);
			if(!strcmp(input, "end"))
				break;
			else{
				realpath(input, filePath);
				printf("%s\n", filePath);
				syncData.nmakedirs++;
				syncData.makedirs = realloc(syncData.makedirs, syncData.nmakedirs * sizeof(char*));
				syncData.makedirs[syncData.nmakedirs - 1] = malloc(strlen(filePath));
				strcpy(syncData.makedirs[syncData.nmakedirs - 1], filePath);
			}
		}
		printf("\nEnter all rc dirs and then enter 'end'\n\n");
		for(;;){
			char touchcommand[256];
			char input[256];
			char filePath[256];
			scanf("%s", &input);
			if(!strcmp(input, "end"))
				break;
			else{
				realpath(input, filePath);
				printf("%s\n", filePath);
				syncData.nrcdirs++;
				syncData.rcdirs = realloc(syncData.rcdirs, syncData.nrcdirs * sizeof(char*));
				syncData.rcdirs[syncData.nrcdirs - 1] = malloc(strlen(filePath));
				strcpy(syncData.rcdirs[syncData.nrcdirs - 1], filePath);

				chdir(currentdir);

				system("touch .syncDatapullrc");
				system("touch .syncDatapushrc");
			}
		}
		printf("writing .syncDatafile\n");
		if(hasFilePath)
			writesyncData(argv[2]);
		else
			writesyncData(".syncDatafile");
		printf("wrote .syncDatafile\n");
	}
}
