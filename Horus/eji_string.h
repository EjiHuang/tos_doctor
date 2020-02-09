#pragma once

#include <iostream>
#include <cstdarg>

class eji_string
{
public:
    std::string format(const char* lpszFormat, ...);
    
};

std::string eji_string::format(const char* lpszFormat, ...)
{
    // Warning : "vsnprintf" crashes with an access violation
    // exception if lpszFormat is not a "const char*" (for example, const string&)

    size_t  nSize = 1024;
    char* lpBuffer = (char*)malloc(nSize);

    va_list lpParams;

    while (true)
    {
        va_start(lpParams, lpszFormat);

        int nResult = vsnprintf(
            lpBuffer,
            nSize,
            lpszFormat,
            lpParams
        );

        va_end(lpParams);

        if ((nResult >= 0) && (nResult < (int)nSize))
        {
            // Success

            lpBuffer[nResult] = '\0';
            std::string sResult(lpBuffer);

            free(lpBuffer);

            return sResult;
        }
        else
        {
            // Increase buffer

            nSize =
                (nResult < 0)
                ? nSize *= 2
                : (nResult + 1)
                ;

            lpBuffer = (char*)realloc(lpBuffer, nSize);
        }
    }
}