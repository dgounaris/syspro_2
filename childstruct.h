typedef struct ChildHN {
	int pid;
	int mid;
	int fid;
	char* dir1; //cdir
	char* dir2; //idir or mdir
	int retries;
	struct ChildHN* next;
} ChildHN;

typedef struct ChildHT {
	ChildHN** table;
	int size;
} ChildHT;

void initChildHT(ChildHT* childHT, int size);

void deleteChildHT(ChildHT* childHT);

void deleteChildHN(ChildHN* childHN);

int getChildHTHash(int pid);

void addChildHN(ChildHT* childHT, int pid, int mid, int fid, char* dir1, char* dir2);

ChildHN* getChildHN(ChildHT* childHT, int pid);

int removeChildHN(ChildHT* childHT, int pid);
