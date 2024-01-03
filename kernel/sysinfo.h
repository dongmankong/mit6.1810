struct sysinfo {
  uint64 freemem;   // amount of free memory (bytes)
  uint64 nproc;     // number of process
};

//my
int getFreeMemNum(void);
int getUnusedProc(void);