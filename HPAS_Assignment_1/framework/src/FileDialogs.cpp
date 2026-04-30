#include "FileDialogs.h"

namespace FileDialogs
{
    char* chooseCSVFile(const char* title, const char* defaultPath)
	{
        const char* csvFileFilter[1] = { "*.csv" };

        return tinyfd_openFileDialog(
            title,
            defaultPath,
            1,
            csvFileFilter,
            "CSV files",
            0
        );

        //char* tinyfd_openFileDialog(
        //    char const* aTitle, /* NULL or "" */
        //    char const* aDefaultPathAndOrFile, /* NULL or "" , ends with / to set only a directory */
        //    int aNumOfFilterPatterns, /* 0 (2 in the following example) */
        //    char const* const* aFilterPatterns, /* NULL or char const * lFilterPatterns[2]={"*.png","*.jpg"}; */
        //    char const* aSingleFilterDescription, /* NULL or "image files" */
        //    int aAllowMultipleSelects); /* 0 or 1 */
        ///* in case of multiple files, the separator is | */
        ///* returns NULL on cancel */
    }
}