#ifndef UTILS_H_
#define UTILS_H_

#include <string.h>

inline int str_equal(const char* str1, const char* str2) {
        return (strcmp(str1, str2) == 0);
}
inline int str_prefix(const char* str, const char* pref) {
        return (strncmp(str, pref, strlen(pref)) == 0);
}
inline int str_suffix(const char* str, const char* pref) {
        return (strncmp(str + strlen(str) - strlen(pref), pref, strlen(pref)) == 0);
}

#endif  // UTILS_H_
