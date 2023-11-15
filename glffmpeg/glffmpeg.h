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

#else

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <malloc.h>
#define GLFFMPEGDECLSPEC
#define __cdecl
#define __stdcall

#endif

/************************************************************************/
/********  GLFFMPEG LIBRARY EXPORTS                                */
/************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

    GLFFMPEGDECLSPEC __cdecl int initializeGLFFMPEG();
    GLFFMPEGDECLSPEC __cdecl int shutdownGLFFMPEG();
    GLFFMPEGDECLSPEC __cdecl int initializeStream(const char* streamName, 
        int fpsRate, int width, int height, void* imageBuffer);
    GLFFMPEGDECLSPEC __cdecl int encodeFrame(const char* streamName); 
    GLFFMPEGDECLSPEC __cdecl int shutdownStream(const char* streamName); 

#ifdef __cplusplus
}
#endif


#endif /* REMOTE_RENDER_H */
