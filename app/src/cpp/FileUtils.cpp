//
// Created by chris on 1/17/17.
//
#include <util/stl.hpp>
#include "FileUtils.h"

#include <sys/stat.h>
#include <fcntl.h>
// #include "tlpi_hdr.h"

#ifndef BUF_SIZE        /* Allow "cc -D" to override definition */
#define BUF_SIZE 1024
#endif

using namespace cs;

bool FileUtils::copyFile(const char* src, const char* dst) {
    int inputFd, outputFd, openFlags;
    mode_t filePerms;
    ssize_t numRead;
    char buf[BUF_SIZE];


    /* Open input and output files */

    inputFd = open(src, O_RDONLY);
    if (inputFd == -1)
        return false;

    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH;      /* rw-rw-rw- */
    outputFd = open(dst, openFlags, filePerms);
    if (outputFd == -1)
        return false;

    /* Transfer data until we encounter end of input or an error */

    while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0)
        if (write(outputFd, buf, numRead) != numRead)
            return false;
    if (numRead == -1)
        return false;

    if (close(inputFd) == -1)
        return false;
    if (close(outputFd) == -1)
        return false;

    return true;
}

bool FileUtils::copyFile(cloture::util::string::String_t src,
                         cloture::util::string::String_t dst) {
    return copyFile(src.getData(), dst.getData());
}