#ifndef COMMON_H
#define COMMON_H

#include "types.h"

string i2s(int number);
int s2i(const string &text);
string currentTime();
string currentTimeMs();
int64_t timestamp();
int64_t timestamp_micro();
bool isFileExists(const string &fileName);
string getFileExt(const string &fileName);
string getStdout(string cmd);
DeviceName detectDevice();
bool readFile(const string &fileName, uint8_t **data, size_t *size);
bool writeFile(const string &fileName, const uint8_t *data, size_t size);
string getCurrentDir();
bool createProcess(const char *path, char *const args[], char *const env[]);
inline void sleepFor(int msec) { std::this_thread::sleep_for(std::chrono::milliseconds(msec)); }
int getVideoDevice();

#endif // COMMON_H
