#ifndef COMMON_H
#define COMMON_H

#include "types.h"

string currentTime();
string currentTimeMs();
int64_t timestamp();
bool isFileExists(const string &fileName);
string getFileExt(const string &fileName);
string getStdout(string cmd);
DeviceName detectDevice();
bool readFile(const string &fileName, uint8_t **data, size_t *size);
bool writeFile(const string &fileName, const uint8_t *data, size_t size);
string getCurrentDir();
bool createProcess(const char *path, char *const args[], char *const env[]);
inline void sleep(int msec) { std::this_thread::sleep_for(std::chrono::milliseconds(msec)); }

#endif // COMMON_H
