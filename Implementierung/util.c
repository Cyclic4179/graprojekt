#include <stdio.h>

/// @brief write into string from float without trailing zeros and without scientific notation
/// should do the same as snprintf(s, "%g", f) for small values
/// @param n max-size of string
/// @param s writable string
/// @param f float value to convert
void ftostr(size_t n, char s[n], float f) {
    int i = snprintf(s, n, "%.6f", f);
    for (int j = i-1; j >= 0; j--) {
        if (s[j] != '0') {
            if (s[j] == '.') {
                s[j] = 0;
            } else {
                s[j+1] = 0;
            }
            break;
        }
    }
}
