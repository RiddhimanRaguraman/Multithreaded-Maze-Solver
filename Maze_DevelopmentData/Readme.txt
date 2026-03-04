1) Build your EXEs for testing
   Make sure you turn on #define FINAL_SUBMIT 1 in main.cpp(line12)

2) Copy your exe and dll to this directory
	example:
		File_DLL_FastX64Release.dll
		MazeX64Release.exe

3) then run the scripts

I found that I need to use:

Maze5x5 - for single stepping code and debugging
Maze500x500 - was more useful as a mini stress test
Maze1Kx1K or greater performed too poorly in Debug mode
Suggest only using Release mode

-Good luck