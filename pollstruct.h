typedef struct PollHN {
	int id;
	int seen;
	struct PollHN* next;
} PollHN;

typedef struct PollHT {
	PollHN** table;
	int size;
} PollHT;

void initPollHT(PollHT* pollHT, int size);

void deletePollHT(PollHT* pollHT);

void deletePollHN(PollHN* pollHN);

int getPollHTHash(int id);

void addPollHN(PollHT* pollHT, int id);

//returns -1 if not found
int getPollHN(PollHT* pollHT, int id);

//returns -1 if not found
int removePollHN(PollHT* pollHT, int id);

void setSeen(PollHT* pollHT, int id);

void resetSeen(PollHT* pollHT);

int nextUnseen(PollHT* pollHT);
