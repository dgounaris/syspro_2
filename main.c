#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utils.h"
#include <unistd.h>
#include "clients.h"
#include "pollstruct.h"
#include <signal.h>
#include "childstruct.h"
#include <fcntl.h>

volatile int sigTerm = 0;

void handleSigT(int sig) {
	sigTerm = 1;
}

void handleSigR(int sig) {
}

int parseArgs(int argc, char* argv[], int* id, char** cDir, char** iDir, char** mDir, int* buffer, char** logFile) {
	int nextArg;
	int idFlag = 1, cDirFlag = 1, iDirFlag = 1, mDirFlag = 1, bFlag = 1, lFileFlag = 1;
	for (nextArg = 1; nextArg < argc; nextArg++) {
		if (strcmp(argv[nextArg], "-n") == 0 && idFlag) {
			if (++nextArg < argc) {
				*id = atoi(argv[nextArg]);
				idFlag = 0;
			} else {
				fprintf(stderr, "Error: Bad parameters format\n");
				return -1;
			}
		} else if (strcmp(argv[nextArg], "-c") == 0 && cDirFlag) {
			if (++nextArg < argc) {
				*cDir = argv[nextArg];
				cDirFlag = 0;
			} else {
				fprintf(stderr, "Error: Bad parameters format\n");
				return -1;
			}
		} else if (strcmp(argv[nextArg], "-i") == 0 && iDirFlag) {
			if (++nextArg < argc) {
				*iDir = argv[nextArg];
				iDirFlag = 0;
			} else {
				fprintf(stderr, "Error: Bad parameters format\n");
				return -1;
			}
		} else if (strcmp(argv[nextArg], "-m") == 0 && mDirFlag) {
			if (++nextArg < argc) {
				*mDir = argv[nextArg];
				mDirFlag = 0;
			} else {
				fprintf(stderr, "Error: Bad parameters format\n");
				return -1;
			}
		} else if (strcmp(argv[nextArg], "-b") == 0 && bFlag) {
			if (++nextArg < argc) {
				*buffer = atoi(argv[nextArg]);
				bFlag = 0;
			} else {
				fprintf(stderr, "Error: Bad parameters format\n");
				return -1;
			}
		} else if (strcmp(argv[nextArg], "-l") == 0 && lFileFlag) {
			if (++nextArg < argc) {
				*logFile = argv[nextArg];
				lFileFlag = 0;
			} else {
				fprintf(stderr, "Error: Bad parameters format\n");
				return -1;
			}
		}
	}
	return 0;
}

void initCommonDir(char* cDir, int id) {
	char* cfPath = malloc((strlen(cDir) + 5)*sizeof(char) + sizeof(int));
	snprintf(cfPath, strlen(cDir) + 5 + sizeof(int), "%s/%d%s", cDir, id, ".id");
	if (fileExists(cfPath)) {
		fprintf(stderr, "ERROR: Client id %d already exists\n", id);
	} else {
		FILE* fp = fopen(cfPath, "w");
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
}

void onSigError(char* mDir, char* cDir, int lFileN, int id, PollHT* pollHT, ChildHT* childHT) {
	fprintf(stdout, "INFO: Poll struct cleanup...\n");
	deletePollHT(pollHT);
	fprintf(stdout, "INFO: Child struct cleanup...\n");
	deleteChildHT(childHT);
	fprintf(stdout, "INFO: Directory cleanup...\n");
	removeDir(mDir);
	fprintf(stdout, "INFO: Mirror directory %s removed successfully\n", mDir);
	char* cfPath = malloc((strlen(cDir) + 5)*sizeof(char) + sizeof(int));
	snprintf(cfPath, strlen(cDir) + 5 + sizeof(int), "%s/%d.id", cDir, id);
	removeFile(cfPath);
	if (lFileN != -1) {
		close(lFileN);
	}
}

int main(int argc, char* argv[]) {
	signal(SIGINT, handleSigT);
	signal(SIGUSR1, handleSigR);
	signal(SIGQUIT, handleSigT);
	char *cDir, *iDir, *mDir;
	char *lFile = NULL;
	int lFileN = -1;
	int id = 1, buffer = 100;
	int pollSize = 1; int childSize = 1;
	if (parseArgs(argc, argv, &id, &cDir, &iDir, &mDir, &buffer, &lFile) == -1) {
		exit(13);
	}
	if (lFile == NULL) {
		fprintf(stdout, "INFO: No log file specified, using standard output\n");
	} else {
		fprintf(stdout, "INFO: Output will be written to file %s\n", lFile);
		lFileN = open(lFile, O_RDWR|O_CREAT|O_TRUNC, 0666);
		if (lFileN == -1) {
			fprintf(stderr, "ERROR: Could not open log file, using standard output\n");
		} else {
			dup2(lFileN, 1);
			dup2(lFileN, 2);
		}
	}
	//validate parameters
	if (iDir == NULL || mDir == NULL || cDir == NULL) {
		fprintf(stderr, "ERROR: Not all required parameters are initialized\n");
		exit(12);
	}
	DIR* inDir = opendir(iDir);
	if (ENOENT == errno) {
		fprintf(stderr, "ERROR: Input directory %s does not exist\n", iDir);
		exit(14);
	} else if (!inDir) {
		fprintf(stderr, "ERROR: Input directory %s could not be opened\n", iDir);
		exit(15);
	}
	DIR* mirDir = opendir(mDir);
	if (mirDir) {
		fprintf(stderr, "ERROR: Mirror directory %s already exists\n", mDir);
		closedir(mirDir);
		exit(16);
	} else if (ENOENT == errno) {
		fprintf(stdout, "INFO: Mirror directory %s will be created\n", mDir);
		if (mkdir(mDir, 0777) == 0) {
			fprintf(stdout, "INFO: Mirror directory %s created\n", mDir);
			mirDir = opendir(mDir);
			if (!mirDir) {
				fprintf(stderr, "ERROR: Mirror directory %s could not be opened\n", mDir);
				exit(18);
			}
		}
	} else {
		fprintf(stderr, "ERROR: Mirror directory %s could not be opened\n", mDir);
		exit(17);
	}
	DIR* comDir = opendir(cDir);
	if (!comDir) {
		fprintf(stdout, "INFO: Common directory %s will be created\n", cDir);
		if (mkdir(cDir, 0777) == 0) {
			fprintf(stdout, "INFO: Common directory %s created\n", cDir);
			comDir = opendir(cDir);
			if (!comDir) {
				fprintf(stderr, "ERROR: Common directory %s could not be opened\n", mDir);
				exit(18);
			}
		}
	} else {
		closedir(comDir);
	}
	initCommonDir(cDir, id);
	//init poll and child struct
	PollHT* pollHT = malloc(sizeof(PollHT));
	initPollHT(pollHT, pollSize);
	ChildHT* childHT = malloc(sizeof(ChildHT));
	initChildHT(childHT, childSize);
	fprintf(stdout, "LOG: CONNECTED ID %d\n", id);
	while (1) {
		//sigint terminating error
		if (sigTerm == 1) {
			fprintf(stdout, "WARN: Terminating signal received, exiting gracefully...\n");
			fprintf(stdout, "LOG: EXITED ID %d\n", id);
			onSigError(mDir, cDir, lFileN, id, pollHT, childHT);
			return 1;
		}
		fprintf(stdout, "INFO: Polling...\n");
		resetSeen(pollHT);
		DIR* dir;
		struct dirent* ent;
		if ((dir = opendir(cDir)) != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				//create a mutable filename copy to search
				char* mutFName = malloc(strlen(ent->d_name)*sizeof(char));
				strcpy(mutFName, ent->d_name);
				char* extension = strrchr(mutFName, '.');
				if (strcmp(extension, ".id") == 0) {
					//change the . to \0 in order to capture the number
					*extension = '\0';
					//this affected mutFName too
					int foundId = atoi(mutFName);
					//ignore self!
					if (foundId == id) {
						continue;
					}
					fprintf(stdout, "INFO: Found id %d...\n", foundId);
					if (getPollHN(pollHT, foundId) == 0) {
						fprintf(stdout, "INFO: Id %d already exists in poll table\n", foundId);
						setSeen(pollHT, foundId);
					} else {
						fprintf(stdout, "INFO: Id %d will be added to poll table...\n", foundId);
						addPollHN(pollHT, foundId);
						setSeen(pollHT, foundId);
						fprintf(stdout, "INFO: Instantiating communication: W %d R %d\n", id, foundId);
						int newwpid = fork();
						if (newwpid < 0) {
							fprintf(stderr, "ERROR: Could not fork child for communication\n");
						}
						else if(newwpid == 0) {
							wClient(id, foundId, cDir, iDir, buffer);
							return 0;
						} else {
							fprintf(stdout, "PARENT MONITOR: Keeping an eye on new child %d\n", newwpid);
							//save the new child to the table
							addChildHN(childHT, newwpid, id, foundId, cDir, iDir);
							int newrpid = fork();
							if (newrpid < 0) {
								fprintf(stderr, "ERROR: Could not fork child for communication\n");
							}
							else if (newrpid == 0) {
								rClient(id, foundId, cDir, mDir);
								return 0;
							} else {
								fprintf(stdout, "PARENT MONITOR: Keeping an eye on new child %d\n", newrpid);
								//save the new child to the table
								addChildHN(childHT, newrpid, id, foundId, cDir, mDir);
							}
						}
					}
				}
				free(mutFName);
			}
			closedir(dir);
		} else {
			fprintf(stderr, "ERROR: Common directory %s could not be opened for polling\n", cDir);
		}
		int unseen;
		while ((unseen = nextUnseen(pollHT)) != -1) {
			fprintf(stdout, "INFO: Id %d is missing, deleting its mirror folder\n", unseen);
			char* misDirF = malloc(strlen(mDir) + 2 + sizeof(int));
			snprintf(misDirF, strlen(mDir)+2+sizeof(int), "%s/%d", mDir, unseen);
			removeDir(misDirF);
			free(misDirF);
		}
		//remove done children from child struct
		int waitstatus;
		int waitres;
		do {
			waitres = waitpid(-1, &waitstatus, WNOHANG);
			if (waitres == 0) {
				fprintf(stdout, "PARENT MONITOR: No processes have finished up\n");
			} else if (waitres > 0) {
				if (WIFEXITED(waitstatus) && WEXITSTATUS(waitstatus) == 0) {
					fprintf(stdout, "PARENT MONITOR: Process with pid %d finished normally\n", waitres);
					removeChildHN(childHT, waitres);
				} else {
					//do retry check and retry here
					ChildHN* childToRetry = getChildHN(childHT, waitres);
					if (childToRetry != NULL) {
						fprintf(stdout, "PARENT MONITOR: Process with pid %d finished abnormally! (retried %d of 3)\n", waitres, childToRetry->retries);
						if (childToRetry->retries < 3) {
							(childToRetry->retries)++;
							fprintf(stdout, "PARENT MONITOR: Retrying process (%d of 3 retries)...\n", childToRetry->retries);
							int forkres = fork();
							if (forkres < 0) {
								fprintf(stderr, "Could not fork child\n");
							} else if (forkres == 0) {
								if (strcmp(iDir, childToRetry->dir2) == 0) {
									wClient(childToRetry->mid, childToRetry->fid, childToRetry->dir1, childToRetry->dir2, buffer);
									return 0;
								} else {
								rClient(childToRetry->mid, childToRetry->fid, childToRetry->dir1, childToRetry->dir2);
									return 0;
								}
							} else {
								(childToRetry->pid) = forkres;
								fprintf(stdout, "PARENT MONITOR: Updated child pid %d\n", forkres);
							}
						}
					} else {
						fprintf(stderr, "PARENT MONITOR: ERROR: Children table has been desynced, observed on pid %d\n", waitres);
					}
				}
			} else {
				fprintf(stdout, "PARENT MONITOR: Error waiting for finished processes\n");
			}
		} while (waitres > 0);
		//retries
		unsigned int sleeptime = 10; //seconds
		//while (sleeptime) {
		sleeptime = sleep(sleeptime);
		//}
	}
	return 0;
}
