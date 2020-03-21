#include <stdio.h>
#include "utils.h"
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

void removeDir(char* path) {
	DIR* dir;
	struct dirent* ent;
	int pathLen;
	if ((dir = opendir(path)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				continue;
			}
			pathLen = strlen(path) + strlen(ent->d_name);
			char* nextPath = malloc((pathLen + 2)*sizeof(char));
			snprintf(nextPath, pathLen+2, "%s/%s", path, ent->d_name);
			fprintf(stdout, "INFO: Deleting node %s...\n", nextPath);
			if (ent->d_type == DT_DIR) {
				fprintf(stdout, "INFO: Recursively delete directory %s...\n", nextPath);
				removeDir(nextPath);
			} else {
				unlink(nextPath);
				fprintf(stdout, "INFO: Deleted file node %s\n", nextPath);
			}
			free(nextPath);
		}
		closedir(dir);
		rmdir(path);
		fprintf(stdout, "INFO: Deleted directory node %s\n", path);
	} else {
		fprintf(stderr, "ERROR: Could not find directory %s to delete\n", path);
	}
}

void removeFile(char* path) {
	unlink(path);
	fprintf(stdout, "INFO: Deleted file %s\n", path);
}

int fileExists(char* path) {
	struct stat* buffer;
	return (stat(path, buffer) == 0);
}
