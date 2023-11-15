The following are instructions for installing glffmpeg to Vega Prime and compiling the source code under
Windows and Linux. Note, for future reference, the following instructions make reference to a folder called 
"Vega_Prime_X". The "X" is refering to the Vega Prime version installed on your machine, like '4' or '5' 
(so Vega_Prime_4 or Vega_Prime_5, etc).

Adding glffmpeg to Vega Prime:
	On Windows:
		1)	Download one of the following files depending on the Visual C/C++ Compiler compiler version
			(or Visual Studio version) used to build your Vega Prime install: 
				VC9 (Visual Studio 2008) or above - glffmpeg-v1.3-win64-x86-vc9-for-vega-prime.zip
				VC9 (Visual Studio 2008) or above - glffmpeg-v1.3-win32-x86-vc9-for-vega-prime.zip
		2)	In the downloaded zip file, grab the ".dll" files and place them in the "bin" directory of 
			your Vega Prime install. This is probably located here:
				C:\Presagis\Vega_Prime_X\bin
			You should now be able to record video in Vega Prime.
			
			Please note that since theses libs are compiled in C the same libs will work with 
			higher versions of Visual Studio like VC10 and higher.
		
	On Linux:
			Please note glffmpeg-v1.3 is currently not supported on Linux but will be available shortly.
			There is little or no change in functionality between glffmpeg-v1.2 and glffmpeg-v1.3.
			The primary purpose of glffmpeg-v1.3 was to bring 64-bit support to Windows using a later x64 capable ffmpeg.
			glffmpeg-v1.2 for Linux already has 64-bit and 32-bit support, please use v1.2 in the mean time.
			glffmpeg-v1.2 for Linux can still be used with the latest Linux VegaPrime version.
			
			
Adding glffmpeg to Vega Prime from Compiled Source Code:
	On Windows:
		1)	Download the file "glffmpeg-v1.3-src.zip" and extract the folder "glffmpeg-v1.3-src".
		2)	In "glffmpeg-v1.3-src", open "glffmpeg_vc9.sln" and compile for Win32 or x64 target depending 
			on your installed Vega Prime VC version.
		3)	In Visual Studio, build the release and debug versions of the glffmpeg project.
		4)	Now grab the dlls out of one of the following folders, again, depending on your installed 
			Vega Prime VC version:
				glffmpeg-v1.3-src\win64\vc9\dll
				glffmpeg-v1.3-src\win32\vc9\dll
			Place those dlls in the "bin" folder of your Vega Prime install which is probably located 
			here:
				C:\Presagis\Vega_Prime_X\bin
		
	On Linux:
			Please note the sources for glffmpeg-v1.3 are currently broken for Linux.
			We are currently trying to fix the issue and an update will be available shortly.
			Please use glffmpeg-v1.2 sources for Linux in the mean time, since it is still 
			compatible with the latest VegaPrime version.
