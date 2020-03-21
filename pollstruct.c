#include <stdio.h>
#include <stdlib.h>
#include "pollstruct.h"

void initPollHT(PollHT* pollHT, int size) {
	pollHT->table = malloc(size*sizeof(PollHN*));
	int i;
	for (i=0;i<size;i++) {
		(*pollHT).table[i] = NULL;
	}
	pollHT->size = size;
}

void deletePollHT(PollHT* pollHT) {
	int i;
	for (i=0;i < pollHT->size;i++) {
		deletePollHN((*pollHT).table[i]);
	}
	free(pollHT);
}

void deletePollHN(PollHN* pollHN) {
	if (pollHN == NULL) {
		return;
	}
	if (pollHN->next != NULL) {
		deletePollHN(pollHN->next);
	}
	free(pollHN);
}

int getPollHTHash(int id) {
	//keeping this as 0 makes this a list
	return 0;
}

void addPollHN(PollHT* pollHT, int id) {
	int index = getPollHTHash(id);
	if ((*pollHT).table[index] == NULL) {
		(*pollHT).table[index] = malloc(sizeof(PollHN));
		((*pollHT).table[index])->id = id;
		((*pollHT).table[index])->seen = 0;
		((*pollHT).table[index])->next = NULL;
	} else {
		PollHN* newhn = (*pollHT).table[index];
		while (newhn->next != NULL) {
			newhn = newhn->next;
		}
		newhn->next = malloc(sizeof(PollHN));
		newhn->next->id = id;
		newhn->next->seen = 0;
		newhn->next->next = NULL;
	}
}

int getPollHN(PollHT* pollHT, int id) {
	int index = getPollHTHash(id);
	PollHN* temp = (*pollHT).table[index];
	while (temp != NULL) {
		if (temp->id == id) {
			return 0;
		}
		temp = temp->next;
	}
	return -1;
}

int removePollHN(PollHT* pollHT, int id) {
	int index = getPollHTHash(id);
	PollHN* temp = (*pollHT).table[index];
	PollHN* prev = NULL;
	while (temp != NULL) {
		if (temp->id == id) {
			if (prev == NULL) {
				free(temp);
				(*pollHT).table[index] = NULL;
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

void setSeen(PollHT* pollHT, int id) {
	int index = getPollHTHash(id);
	PollHN* temp = (*pollHT).table[index];
	while (temp != NULL) {
		if (temp->id == id) {
			temp->seen = 1;
			return;
		}
		temp = temp->next;
	}
}

void resetSeen(PollHT* pollHT) {
	int i=0;
	for (i=0;i < pollHT->size; i++) {
		PollHN* temp = (*pollHT).table[i];
		while (temp != NULL) {
			temp->seen = 0;
			temp = temp->next;
		}
	}
}

int nextUnseen(PollHT* pollHT) {
	int i=0;
	for (i=0;i < pollHT->size; i++) {
		PollHN* temp = (*pollHT).table[i];
		while (temp != NULL) {
			if (temp->seen == 0) {
				temp->seen = 1;
				return temp->id;
			}
			temp = temp->next;
		}
	}
	return -1;
}
