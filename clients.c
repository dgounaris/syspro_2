#include <stdio.h>
#include <stdlib.h>
#include "clients.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "utils.h"
#include <signal.h>

void wClient(int myid, int forid, char* cDir, char* iDir, int buffer) {
	int fullpathsize = (strlen(cDir)+11)*sizeof(char) + 2*sizeof(int);
	//exit(6);
	char* fifoname = malloc(fullpathsize);
	snprintf(fifoname, fullpathsize, "%s/%d_to_%d.fifo", cDir, myid, forid);
	int mkresult = mkfifo(fifoname, 0666);
	if (mkresult == 0) {
		fprintf(stdout, "INFO: Named pipe %s created\n", fifoname);
	} else if (mkresult == -1 && EEXIST == errno) {
		fprintf(stdout, "INFO: Named pipe %s exists, opening it instead\n", fifoname);
	} else {
		fprintf(stderr, "ERROR: Could not create pipe %s\n", fifoname);
		kill(getppid(), SIGUSR1);
		exit(7);
	}
	int fd = open(fifoname, O_WRONLY);
	fprintf(stdout, "INFO: WRITE START for pipe %s\n", fifoname);
	wAction(fd, iDir, "", buffer);
	//finish writing by 00
	unsigned short finished = 0;
	if (write(fd, &finished, 2) == 2) {
		fprintf(stdout, "INFO: End of input appended successfully\n");
	}
	fprintf(stdout, "INFO: WRITE END for pipe %s\n", fifoname);
	close(fd);
	//removeFile(fifoname);
}

void alarmSigHandler(int sig) {
	kill(getppid(), SIGUSR1);
	exit(9);
}

void rClient(int myid, int forid, char* cDir, char* mDir) {
	signal(SIGALRM, alarmSigHandler);
	int fullpathsize = (strlen(cDir)+11)*sizeof(char) + 2*sizeof(int);
	char* fifoname = malloc(fullpathsize);
	snprintf(fifoname, fullpathsize, "%s/%d_to_%d.fifo", cDir, forid, myid);
	int mkresult = mkfifo(fifoname, 0666);
	if (mkresult == 0) {
		fprintf(stdout, "INFO: Named pipe %s created\n", fifoname);
	} else if (mkresult == -1 && EEXIST == errno) {
		fprintf(stdout, "INFO: Named pipe %s exists, opening it instead\n", fifoname);
	} else {
		fprintf(stderr, "ERROR: Could not create pipe %s\n", fifoname);
		kill(getppid(), SIGUSR1);
		exit(7);
	}
	//terminate after 30 seconds
	alarm(30);
	int fd = open(fifoname, O_RDONLY);
	fprintf(stdout, "INFO: READ START for pipe %s\n", fifoname);
	int response = rAction(fd, mDir, forid);
	close(fd);
	if (response == 0) {
		fprintf(stdout, "INFO: Read pipe successfully\n");
	} else {
		fprintf(stderr, "ERROR: Error while reading pipe\n");
		kill(getppid(), SIGUSR1);
		exit(8);
	}
	close(fd);
	removeFile(fifoname);
}

//realPath/path parameters mask the input folder to the recipient
void wAction(int fd, char* realPath, char* path, int buffer) {
	unsigned short fnsize = 0;
	int fsize = 0;
	DIR* dir;
	struct dirent* ent;
	int pathLen;
	int pathRealLen;
	if ((dir = opendir(realPath)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				continue;
			}
			pathLen = strlen(path) + strlen(ent->d_name);
			pathRealLen += strlen(realPath) + strlen(ent->d_name);
			char* nextRealPath = malloc((pathRealLen+2)*sizeof(char));
			char* nextPath = malloc((pathLen+2)*sizeof(char));
			snprintf(nextPath, pathLen + 2, "%s/%s", path, ent->d_name);
			snprintf(nextRealPath, pathRealLen + 2, "%s/%s", realPath, ent->d_name);
			if (ent->d_type == DT_DIR) {
				fprintf(stdout, "INFO: Directory %s found, writing to pipe recursively...\n", nextPath);
				wAction(fd, nextRealPath, nextPath, buffer);
			} else {
				fprintf(stdout, "INFO: File %s found (masked name %s), writing to pipe...\n", nextRealPath, nextPath);
				//first filename size
				fnsize = strlen(nextPath);
				if (write(fd, &fnsize, 2) == 2) {
					fprintf(stdout, "INFO: File name size %hu written successfully\n", fnsize);
				}
				//write name without trailing \0
				if (write(fd, nextPath, pathLen+1) == pathLen+1) {
					fprintf(stdout, "INFO: File name %s written successfully\n", nextPath);
				}
				//write file size
				fprintf(stdout, "INFO: Opening file %s to write...\n", nextRealPath);
				FILE* fp = fopen(nextRealPath, "r");
				fseek(fp, 0L, SEEK_END);
				fsize = ftell(fp);
				rewind(fp);
				if (write(fd, &fsize, 4) == 4) {
					fprintf(stdout, "INFO: File size %d written successfully\n", fsize);
				}
				//write file data
				char* filedata = malloc(fsize*sizeof(char));
				fread(filedata, sizeof(char), fsize, fp);
				//apply buffered writing
				int tempfsize = 0;
				int slotsize;
				//if (write(fd, filedata, fsize) != fsize) {
				//	fprintf(stderr, "ERROR: Error while writing to pipe\n");
				//}
				while (tempfsize < fsize) {
					int wfsize = tempfsize;
					if (tempfsize + buffer > fsize) {
						slotsize = fsize - tempfsize;
						tempfsize += slotsize;
					} else {
						slotsize = buffer;
						tempfsize += buffer;
					}
					if (write(fd, &(filedata[wfsize]), slotsize) == slotsize) {
						fprintf(stdout, "INFO: Buffered writing...\n");
					}
				}
				fprintf(stdout, "INFO: File data written successfully\n");
				fprintf(stdout, "LOG: WRITE %s %d\n", nextPath, 6 + fsize + fnsize);
				free(filedata);
				fclose(fp);
				fprintf(stdout, "INFO: Closed file %s\n", nextRealPath);
			}
			free(nextPath);
			free(nextRealPath);
		}
		closedir(dir);
	}
}

void alarmSigSilencer(int sig) {
	//do nothing since here it means that the process is actively reading
}

int rAction(int fd, char* path, int forid) {
	int totalpathsize = strlen(path) + sizeof(int) + 2;
	char* totalpath = malloc(totalpathsize);
	snprintf(totalpath, totalpathsize, "%s/%d", path, forid);
	DIR* rDir = opendir(totalpath);
	if (rDir) {
		fprintf(stdout, "INFO: Directory %s already existed, writing in it\n", totalpath);
		closedir(rDir);
	} else if (ENOENT == errno) {
		if (mkdir(totalpath, 0777) == 0) {
			fprintf(stdout, "INFO: Directory %s created successfully\n", totalpath);
		} else {
			fprintf(stderr, "ERROR: Could not create directory %s\n", totalpath);
			return -2;
		}
	}
	unsigned short fnsize = 0;
	int fsize = 0;
	int rresult = read(fd, &fnsize, 2);
	//we are past the first read, we can safely assume that we are reading
	signal(SIGALRM, alarmSigSilencer);
	while (fnsize != 0) {
		if (rresult == 2) {
			fprintf(stdout, "INFO: File name size %hu read successfully\n", fnsize);
		}
		char* fname = malloc(fnsize+1);
		if (read(fd, fname, fnsize) == fnsize) {
			fname[fnsize] = '\0';
			fprintf(stdout, "INFO: File name %s read successfully\n", fname);
		}
		if (read(fd, &fsize, 4) == 4) {
			fprintf(stdout, "INFO: File size %d read successfully\n", fsize);
		}
		char* fdata = malloc(fsize);
		int uptonowR = 0;
		int nowR = 0;
		while (uptonowR < fsize) {
			nowR = read(fd, &(fdata[uptonowR]), fsize-uptonowR);
			if (nowR == -1 || nowR == 0) {
				fprintf(stderr, "ERROR: Could not read from pipe\n");
				return -1;
			} else {
				uptonowR += nowR;
			}
		}
		if (uptonowR == fsize) {
			fprintf(stdout, "INFO: File data read successfully\n");
		}
		fprintf(stdout, "LOG: READ %s %d\n", fname, 6 + fnsize + fsize);
		fprintf(stdout, "INFO: Creating necessary directory hierarchy for %s...\n", fname);
		char* fnamecp = malloc(strlen(fname)+1);
		snprintf(fnamecp, strlen(fname)+1, "%s", fname);
		createRDir(totalpath, fname);
		char* filedest = malloc(strlen(totalpath)+strlen(fnamecp)+1);
		snprintf(filedest, strlen(totalpath) + strlen(fnamecp) + 1, "%s%s", totalpath, fnamecp);
		fprintf(stdout, "INFO: Opening file %s to copy...\n", filedest);
		FILE* fp = fopen(filedest, "w");
		fwrite(fdata, sizeof(char), fsize, fp);
		fclose(fp);
		free(fnamecp);
		free(filedest);
		free(fname);
		free(fdata);
		rresult = read(fd, &fnsize, 2);
	}
	free(totalpath);
	return 0;
}

void createRDir(char* root, char* filepath) {
	char* token = strtok(filepath, "/");
	if (token == NULL) {
		fprintf(stdout, "WARN: Empty filepath passed to reader for dir %s\n", root);
		return;
	}
	char* lookuph = strtok(NULL, "/");
	char* newdirname = malloc(strlen(root) + strlen(token) + 2);
	snprintf(newdirname, strlen(root) + strlen(token) + 2, "%s/%s", root, token);
	while (lookuph != NULL) {
		DIR* dir = opendir(newdirname);
		if (dir) {
			fprintf(stdout, "INFO: Directory %s already exists in %s\n", newdirname, root);
			closedir(dir);
		} else if (ENOENT == errno) {
			if (mkdir(newdirname, 0777) == 0) {
				fprintf(stdout, "INFO: Created directory %s in %s\n", newdirname, root);
			} else {
				fprintf(stderr, "ERROR: Error creating directory %s in %s\n", newdirname, root);
			}
		} else {
			fprintf(stderr, "ERROR: Directory %s could not be opened in %s\n", newdirname, root);
		}
		char* newdirnameold = newdirname;
		newdirname = malloc(strlen(newdirnameold) + strlen(lookuph) + 2);
		snprintf(newdirname, strlen(newdirnameold) + strlen(lookuph) + 2, "%s/%s", newdirnameold, lookuph);
		free(newdirnameold);
		lookuph = strtok(NULL, "/");
	}
	free(newdirname);
}
