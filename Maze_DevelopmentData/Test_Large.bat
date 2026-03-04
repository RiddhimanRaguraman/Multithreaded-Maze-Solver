

del tresults*.txt

.\MazeX64Release.exe Maze2kx2k.data > tresults_2k.txt 
.\MazeX64Release.exe Maze5kx5k.data > tresults_5k.txt 
.\MazeX64Release.exe Maze10kx10k.data > tresults_10k.txt
.\MazeX64Release.exe Maze15kx15k.data > tresults_15k.txt

type tresults_2k.txt tresults_5k.txt tresults_10k.txt tresults_15K.txt > results_large.txt


del tresults*.txt
 