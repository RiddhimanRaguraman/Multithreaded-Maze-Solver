

del tresults*.txt

.\MazeX64Release.exe Maze100x100.data > tresults_100.txt 
.\MazeX64Release.exe Maze200x200.data > tresults_200.txt 
.\MazeX64Release.exe Maze500x500.data > tresults_500.txt
.\MazeX64Release.exe Maze1Kx1K.data > tresults_1K.txt

type tresults_100.txt tresults_200.txt tresults_500.txt tresults_1K.txt > results_medium.txt


del tresults*.txt
 