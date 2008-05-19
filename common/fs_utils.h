#ifndef FS_UTILS_H
#define FS_UTILS_H
#include <string>
#include <unistd.h>

bool check_filename(std::string f);
bool check_filemode(mode_t m);

#endif
