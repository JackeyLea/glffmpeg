/*
* Main header for the glFFmpeg library
* Copyright (c) 2006 Marco Ippolito
*
* This file is part of glFFmpeg.
*/

#ifndef GLFFMPEG_H
#define GLFFMPEG_H

#ifdef _WIN32

#include <windows.h>
#define GLFFMPEGDECLSPEC __declspec(dllexport)
#define WIN32_LEAN_AND_MEAN 1

#else // Linux/Unix

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <malloc.h>
#define GLFFMPEGDECLSPEC
#define __cdecl
#define __stdcall

#endif // _WIN32

//------------------------------------------------------------------------------
// GLFFMPEG LIBRARY EXPORTS
//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
{
#endif

    /**
     * Each function exposed by the DLL will return one of the possible status 
     * error codes below:
     *
     * 0 Success - status normal.
     * 1 ffmpegHelper::configure: Unable to locate a suitable encoding format.
     * 2 ffmpegHelper::configure: Error allocating video context.
     * 3 ffmpegHelper::configure: Error allocating video stream.
     * 4 ffmpegHelper::configure: Invalid output parameters.
     * 5 ffmpegHelper::configure: Codec not found.
     * 6 ffmpegHelper::configure: Could not open codec.
     * 7 ffmpegHelper::configure: Error allocating video frame.
     * 8 ffmpegHelper::configure: Unable to open output file <file_name>,
     * 9 ffmpegHelper::encodeFrame: You need to create a stream first.
     *
     * Use the getStatus() function to check the status of a particlaur stream.
     */

    int GLFFMPEGDECLSPEC __cdecl initializeGLFFMPEG();
    int GLFFMPEGDECLSPEC __cdecl shutdownGLFFMPEG();
    int GLFFMPEGDECLSPEC __cdecl initializeStream(const char* streamName, 
        int fpsRate, int width, int height, void* imageBuffer);
    int GLFFMPEGDECLSPEC __cdecl encodeFrame(const char* streamName); 
    int GLFFMPEGDECLSPEC __cdecl shutdownStream(const char* streamName);
    int GLFFMPEGDECLSPEC __cdecl getStatus(const char* streamName);

#ifdef __cplusplus
}
#endif

#endif // REMOTE_RENDER_H
