#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "childstruct.h"

void initChildHT(ChildHT* childHT, int size) {
	childHT->table = malloc(size*sizeof(ChildHN*));
	int i;
	for (i=0;i<size;i++) {
		(*childHT).table[i] = NULL;
	}
	childHT->size = size;
}

void deleteChildHT(ChildHT* childHT) {
	int i;
	for (i=0;i < childHT->size;i++) {
		deleteChildHN((*childHT).table[i]);
	}
	free(childHT);
}

void deleteChildHN(ChildHN* childHN) {
	if (childHN == NULL) {
		return;
	}
	if (childHN->next != NULL) {
		deleteChildHN(childHN->next);
	}
	free(childHN);
}

int getChildHTHash(int id) {
	//keeping this 0 makes it a list
	//cant be an easy hash because the pid of a child may change due to retries
	return 0;
}

void addChildHN(ChildHT* childHT, int pid, int mid, int fid, char* dir1, char* dir2) {
	int index = getChildHTHash(pid);
	if ((*childHT).table[index] == NULL) {
		(*childHT).table[index] = malloc(sizeof(ChildHN));
		((*childHT).table[index])->pid = pid;
		((*childHT).table[index])->mid = mid;
		((*childHT).table[index])->fid = fid;
		((*childHT).table[index])->dir1 = malloc(strlen(dir1)+1);
		strcpy(((*childHT).table[index])->dir1, dir1);
		((*childHT).table[index])->dir2 = malloc(strlen(dir2)+1);
		strcpy(((*childHT).table[index])->dir2, dir2);
		((*childHT).table[index])->retries = 0;
		((*childHT).table[index])->next = NULL;
	} else {
		ChildHN* newhn = (*childHT).table[index];
		while (newhn->next != NULL) {
			newhn = newhn->next;
		}
		newhn->next = malloc(sizeof(ChildHN));
		newhn->next->pid = pid;
		newhn->next->mid = mid;
		newhn->next->fid = fid;
		newhn->next->dir1 = malloc(strlen(dir1)+1);
		strcpy(newhn->next->dir1, dir1);
		newhn->next->dir2 = malloc(strlen(dir2)+1);
		strcpy(newhn->next->dir2, dir2);
		newhn->next->retries = 0;
		newhn->next->next = NULL;
	}
}

ChildHN* getChildHN(ChildHT* childHT, int pid) {
	int index = getChildHTHash(pid);
	ChildHN* temp = (*childHT).table[index];
	while (temp != NULL) {
		if (temp->pid == pid) {
			return temp;
		}
		fprintf(stdout, "searching for %d\n", pid);
		fprintf(stdout, "passed by %d\n", temp->pid);
		temp = temp->next;
	}
	return NULL;
}

int removeChildHN(ChildHT* childHT, int pid) {
	int index = getChildHTHash(pid);
	ChildHN* temp = (*childHT).table[index];
	ChildHN* prev = NULL;
	while (temp != NULL) {
		if (temp->pid == pid) {
			if (prev == NULL) {
				(*childHT).table[index] = ((*childHT).table[index])->next;
				free(temp);
			} else {
				prev->next = temp->next;
				free(temp);
			}
			return 0;
		}
		prev = temp;
		temp = temp->next;
	}
	return -1;
}
