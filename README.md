# Multithreaded Maze Solver

A C++ maze solver framework designed to experiment with concurrency on a large search problem, utilizing lock-free synchronization and atomic operations for high-performance parallel processing.

**Performance Snapshot**:
For a **15,000 x 15,000** maze, this multithreaded solution reduced the execution time from **5.96s (BFS)** to **1.10s**, achieving a **~5.4x speedup**.

## 1. Problem Overview
The application is a C++ maze solver framework designed to experiment with concurrency on a large search problem.
*   **Maze Structure**: A maze is stored as a 2D grid of cells in the `Maze` class.
*   **Data Storage**: Backed by an array of `std::atomic_uint` values (`poMazeData`) and loaded from a file.
*   **Goal**: The start position is in the middle of the top row and the goal is in the middle of the bottom row.
*   **Movement**: Valid moves from any position are encoded as "openings" to the north, south, east, or west.

## 2. Solution Approach
This solver uses bidirectional exploration with two threads running depth-first traversal from the Start and the End.
*   **Bidirectional Search**: Two threads explore simultaneously, one from the Start (Top-Down) and one from the End (Bottom-Up).
*   **Branch Skipping**: Each thread performs branch skipping in corridors to reduce frontier size.
*   **Route Provenance**: Marks route provenance into disjoint bit fields in the shared atomic maze array.
*   **Meet Detection**: Threads detect a meet point via per-thread visited bits.
*   **Termination**: Threads terminate as soon as they meet or reach the opposite goal.
*   **Path Stitching**: The final path is stitched from the recorded route bits after a meet is detected.

## 3. Key Implementation Highlights
*   **Thread Creation**: Threads are created in `MTMazeStudentSolver::Solve()`. The main thread constructs two `std::thread` instances (`solveWorkerTD` and `solveWorkerBU`) and later joins them.
*   **Worker Logic**: Each worker maintains a local stack of `Choice` entries, advances using `SkippingMazeSolver::follow`, marks route bits per step, and sets a per-thread visited bit using `tryVisitDual`.
*   **Lock-Free Synchronization**: Cell-level communication is lock-free via `std::atomic_uint` with `fetch_or` to set per-thread visited bits and detect meets. **No mutexes are used.**
*   **Coordination**: Signaling uses `std::atomic_bool found` to coordinate early termination.
*   **Memory Ordering**:
    *   **Relaxed Ordering**: Used for visited marking (`fetch_or`) because operations are idempotent and independent.
    *   **Acquire-Release**: Used for publishing `found` status to ensure visibility of prior route writes across threads.
    *   **Sequential Consistency**: Used in exceptional finalization paths for debugging assurance.
*   **Shared Scope**: The shared scope is the `Maze::poMazeData` array of `std::atomic_uint` cells.

## 4. How to Run

### Option A: Development Data (Testing Environment)
The `Maze_DevelopmentData` folder contains the testing environment with an existing version of the project executable and test scripts.
1.  Navigate to the `Maze_DevelopmentData` folder.
2.  Run the `Test_Contest.bat` file to execute the solver against the contest dataset.

### Option B: Visual Studio Solution (Source Code)
The `Maze_Solution` folder contains the source code and Visual Studio solution.
1.  Open `Maze_Solution\Maze.sln` in Visual Studio.
2.  Ensure the build configuration is set to **Release** mode (x64).
3.  To manually run a specific maze:
    *   Open `Maze_Solution\Maze\main.cpp`.
    *   Locate lines 23-26:
        ```cpp
        #if !FINAL_SUBMIT
            //char inFileName[INPUT_NAME_SIZE] = "Maze15kx15K_E.data";
            char inFileName[INPUT_NAME_SIZE] = "Maze20kx20K_D.data";
            //char inFileName[INPUT_NAME_SIZE] = "Maze1kx1K.data";
        ```
    *   Set `#define FINAL_SUBMIT 0` (if not already set) to enable manual file selection.
    *   Uncomment the desired maze file or modify the filename string.
4.  Build and Run the project.

## 5. Performance Results
The following results demonstrate the performance improvements achieved by the multithreaded student solver compared to single-threaded BFS and DFS implementations.

```text
****************************************
**      Framework: 4.43               **
**   C++ Compiler: 194334810          **
**  Tools Version: 14.43.34808        **
**    Windows SDK: 10.0.26100.0       **
**   Mem Tracking: --> DISABLED <--   **
**           Mode: x64 Release        **
****************************************
**         Thread: 1.31               **
****************************************

Maze: start(.\Maze15Kx15K_E.data) ------------

 Maze: STMazeSolverBFS
    checkSolution(172359 elements): passed

 Maze: STMazeSolverDFS
    checkSolution(172359 elements): passed

 Maze: MTStudentSolver
    checkSolution(172359 elements): passed

Results (.\Maze15Kx15K_E.data):

   BFS      : 5.961873 s
   DFS      : 3.046977 s
   Student  : 1.102652 s

Maze: end() --------------


****************************************

****************************************
**      Framework: 4.43               **
**   C++ Compiler: 194334810          **
**  Tools Version: 14.43.34808        **
**    Windows SDK: 10.0.26100.0       **
**   Mem Tracking: --> DISABLED <--   **
**           Mode: x64 Release        **
****************************************
**         Thread: 1.31               **
****************************************

Maze: start(.\Maze15Kx15K_J.data) ------------

 Maze: STMazeSolverBFS
    checkSolution(164043 elements): passed

 Maze: STMazeSolverDFS
    checkSolution(164043 elements): passed

 Maze: MTStudentSolver
    checkSolution(164043 elements): passed

Results (.\Maze15Kx15K_J.data):

   BFS      : 5.403713 s
   DFS      : 3.888267 s
   Student  : 1.293958 s

Maze: end() --------------


****************************************

****************************************
**      Framework: 4.43               **
**   C++ Compiler: 194334810          **
**  Tools Version: 14.43.34808        **
**    Windows SDK: 10.0.26100.0       **
**   Mem Tracking: --> DISABLED <--   **
**           Mode: x64 Release        **
****************************************
**         Thread: 1.31               **
****************************************

Maze: start(.\Maze20Kx20K_B.data) ------------

 Maze: STMazeSolverBFS
    checkSolution(306443 elements): passed

 Maze: STMazeSolverDFS
    checkSolution(306443 elements): passed

 Maze: MTStudentSolver
    checkSolution(306443 elements): passed

Results (.\Maze20Kx20K_B.data):

   BFS      : 9.566754 s
   DFS      : 3.616433 s
   Student  : 3.642013 s

Maze: end() --------------


****************************************

****************************************
**      Framework: 4.43               **
**   C++ Compiler: 194334810          **
**  Tools Version: 14.43.34808        **
**    Windows SDK: 10.0.26100.0       **
**   Mem Tracking: --> DISABLED <--   **
**           Mode: x64 Release        **
****************************************
**         Thread: 1.31               **
****************************************

Maze: start(.\Maze20Kx20K_D.data) ------------

 Maze: STMazeSolverBFS
    checkSolution(291601 elements): passed

 Maze: STMazeSolverDFS
    checkSolution(291601 elements): passed

 Maze: MTStudentSolver
    checkSolution(291601 elements): passed

Results (.\Maze20Kx20K_D.data):

   BFS      : 8.987570 s
   DFS      : 4.191192 s
   Student  : 2.138222 s

Maze: end() --------------


****************************************
```
