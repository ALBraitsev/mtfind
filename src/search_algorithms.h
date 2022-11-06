#pragma once

#include <algorithm>

/// @brief 

typedef std::function<const char *(const char *string, int stringSize,
                                   const char *pattern, int patternSize)>
    SerchAlgorithm;

//////////////////////////////////////////////////////////////////////////////// @brief
///
/// @param string
/// @param stringSize
/// @param pattern
/// @param patternSize
/// @return const char*
inline const char *bruteforcematch(const char *string, int stringSize,
                                   const char *pattern, int patternSize)
{
    if (patternSize > stringSize)
        return nullptr;

    int i, j;
    for (j = 0; j <= stringSize - patternSize; ++j)
    {
        for (i = 0; i < patternSize && (pattern[i] == '?' || pattern[i] == string[i + j]); ++i)
            ;
        if (i >= patternSize)
            return string + j;
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////// @brief
///

#define NO_OF_CHARS 127

// A utility function to get maximum of two integers
inline int max(int a, int b) { return (a > b) ? a : b; }

// The preprocessing function for Boyer Moore's
// bad character heuristic
inline void badCharHeuristic(const char *str, int size,
                             int badchar[NO_OF_CHARS])
{
    int i;

    // Initialize all occurrences as -1
    for (i = 0; i < NO_OF_CHARS; i++)
        badchar[i] = -1;

    // Fill the actual value of last occurrence
    // of a character
    for (i = 0; i < size; i++)
        badchar[(int)str[i]] = i;
}

struct Moore
{
    int badchar[NO_OF_CHARS];

    Moore(const char *pat, int m)
    {
        /* Fill the bad character array by calling
        the preprocessing function badCharHeuristic()
        for given pattern */
        badCharHeuristic(pat, m, badchar);
    }

    const char *operator()(const char *txt, int n, const char *pat, int m)
    {
        if (m > n)
            return nullptr;

        int s = 0; // s is shift of the pattern with
                   // respect to text
        while (s <= (n - m))
        {
            int j = m - 1;

            /* Keep reducing index j of pattern while
            characters of pattern and text are
            matching at this shift s */
            while (j >= 0 && (pat[j] == '?' || pat[j] == txt[s + j]))
                j--;

            /* If the pattern is present at current
            shift, then index j will become -1 after
            the above loop */
            if (j < 0)
            {
                return txt + s;

                /* Shift the pattern so that the next
                character in text aligns with the last
                occurrence of it in pattern.
                The condition s+m < n is necessary for
                the case when pattern occurs at the end
                of text */
                // s += (s+m < n)? m-badchar[txt[s+m]] : 1;
            }

            else
                /* Shift the pattern so that the bad character
                in text aligns with the last occurrence of
                it in pattern. The max function is used to
                make sure that we get a positive shift.
                We may get a negative shift if the last
                occurrence of bad character in pattern
                is on the right side of the current
                character. */
                s += pat[m - j - 1] == '?' ? 1 : max(1, j - badchar[txt[s + j]]);
        }
        return nullptr;
    }
};
