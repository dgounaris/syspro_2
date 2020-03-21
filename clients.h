void wClient(int myid, int forid, char* cDir, char* iDir, int buffer);

void wAction(int fd, char* realPath,  char* path, int buffer);

void rClient(int myid, int forid, char* cDir, char* mDir);

int rAction(int fd, char* path, int forid);

void createRDir(char* root, char* filepath);
