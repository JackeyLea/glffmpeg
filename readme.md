## GLFFmpeg

### Documentation

GLFFmpeg uses the video recording functionality provided by the FFmpeg library to capture content generated using OpenGL.
	
The GLFFmpeg library provides a minimalistic set of entry points into the (very thorough) FFmpeg API to simplify the task of capturing OpenGL output to a video stream.

See the samples included with the distribution for usage.

Pre-built binaries and Instructions for building ffmpeg on 

Windows can be found at: https://github.com/BtbN/FFmpeg-Builds/releases

The latest ffmpeg sources can be found at: https://github.com/FFmpeg/FFmpeg

The following are instructions for installing glffmpeg to Vega Prime and compiling the source code under Windows and Linux. Note, for future reference, the following instructions make reference to a folder called "Vega_Prime_X". The "X" is refering to the Vega Prime version installed on your machine, like '3' or '4' (so Vega_Prime_3 or Vega_Prime_4, etc).

Adding glffmpeg to Vega Prime:

On Windows:

1)	Download one of the following files depending on the Visual C/C++ Compiler compiler version (or Visual Studio version) used to build your Vega Prime install: 
VC8 (Visual Studio 2005)	-	glffmpeg-v1.2-win32-x86-vc8-for-vega-prime.zip
VC9 (Visual Studio 2008)	-	glffmpeg-v1.2-win32-x86-vc9-for-vega-prime.zip

2)	In the downloaded zip file, grab the ".dll" files and place them in the "bin" directory of your Vega Prime install. This is probably located here:
C:\Presagis\Vega_Prime_X\bin

You should now be able to record video in Vega Prime.
		
On Linux:

1)	Download one of the following files depending on which architecture of Vega Prime you are running:

32 bit	-	glffmpeg-v1.2-linux-x86-for-vega-prime.tar.gz
64 bit	-	glffmpeg-v1.2-linux-x86-64-for-vega-prime.tar.gz

2)	In the download tar.gz file, grab the "libglffmpeg.so" file and place it in the "lib" or "lib64" (depending on your machine's architecture) directory of your Vega Prime install. These directories are probably located under this directory: /usr/local/Presagis/Vega_Prime_X

You should now be able to record video in Vega Prime.
			
Adding glffmpeg to Vega Prime from Compiled Source Code:

On Windows:
1)	Download the file "glffmpeg-v1.2-src.zip" and extract the folder "glffmpeg-v1.2-src".
2)	In "glffmpeg-v1.2-src", open either "glffmpeg_vc8.sln" or "glffmpeg_vc9.sln" depending on your installed Vega Prime VC version.
3)	In Visual Studio, build the release and debug versions of the glffmpeg project.
4)	Now grab the dlls out of one of the following folders, again, depending on your installed Vega Prime VC version:
	glffmpeg-v1.2-src\win32\vc8\dll
	glffmpeg-v1.2-src\win32\vc9\dll

Place those dlls in the "bin" folder of your Vega Prime install which is probably located here: C:\Presagis\Vega_Prime_X\bin

On Linux:

1)	Download the file "glffmpeg-v1.2-src.tar.gz" and extract the folder "glffmpeg-v1.2-src".
2)	Open a terminal and cd to the "glffmpeg-v1.2-src" folder. Then do the following based on the architecture of your machine:

32 bit:
1)	Run the following commands in your open terminal:

```bash
./configure
make
```

2) 	Place the "libglffmpeg" library (it should be in the "glffmpeg" folder) in the "lib" folder of your Vega Prime install (probably located under "/usr/local/Presagis/Vega_Prime_X").

64 bit:
1)	Go to the folder "glffmpeg-v1.2-src\glffmpeg\" and open the "Makefile". 
	Remove "-m32" from the command line options of "$(CC)" (near the bottom).
2)	Go back to the folder "glffmpeg-v1.2-src" and run the following commands in your terminal:

```bash
./configure --extra-cflags=-fPIC
make
```

3) 	Place the "libglffmpeg" library (it should be in the "glffmpeg" folder) in the "lib64" folder of your Vega Prime install (probably located under "/usr/local/Presagis/Vega_Prime_X").
