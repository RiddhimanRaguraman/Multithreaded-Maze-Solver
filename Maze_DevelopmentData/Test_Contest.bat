

del tresults*.txt

.\MazeX64Release.exe .\Maze15Kx15K_E.data > tresultsE.txt 
.\MazeX64Release.exe .\Maze15Kx15K_J.data > tresultsJ.txt 
.\MazeX64Release.exe .\Maze20Kx20K_B.data > tresultsB.txt
.\MazeX64Release.exe .\Maze20Kx20K_D.data > tresultsD.txt

type tresultsE.txt tresultsJ.txt tresultsB.txt tresultsD.txt > results_contest.txt

del tresults*.txt