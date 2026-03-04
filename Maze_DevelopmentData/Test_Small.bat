

del tresults*.txt

.\MazeX64Release.exe Maze5x5.data > tresults_5.txt 
.\MazeX64Release.exe Maze6x6.data > tresults_6.txt 
.\MazeX64Release.exe Maze10x10.data > tresults_10.txt
.\MazeX64Release.exe Maze20x20.data > tresults_20.txt
.\MazeX64Release.exe Maze50x50.data > tresults_50.txt

type tresults_5.txt tresults_6.txt tresults_10.txt tresults_20.txt tresults_50.txt> results_small.txt

del tresults*.txt

 