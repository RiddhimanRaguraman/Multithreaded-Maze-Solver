//------------------------------------------------------
// Copyright 2025, Ed Keenan, all rights reserved.
//------------------------------------------------------

#ifndef MT_MAZE_STUDENT_SOLVER_H
#define MT_MAZE_STUDENT_SOLVER_H

#include "Choice.h"
#include "SkippingMazeSolver.h"

class MTMazeStudentSolver : public SkippingMazeSolver 
{
private:
    std::vector<Direction>* solution;
    std::atomic_bool found;
    char pad_[7];

public: 
    MTMazeStudentSolver(Maze *maze) 
    : SkippingMazeSolver( maze ), solution(nullptr), found(false)
    {
        assert(pMaze);
    }

    MTMazeStudentSolver() = delete;
    MTMazeStudentSolver(const MTMazeStudentSolver &) = delete;
    MTMazeStudentSolver &operator=(const MTMazeStudentSolver &) = delete;
    virtual ~MTMazeStudentSolver() = default;

    std::vector<Direction> *Solve() override
    {
        solution = nullptr;
        found.store(false, std::memory_order_relaxed);

        std::thread TD = std::thread(&MTMazeStudentSolver::solveWorkerTD, this);
        std::thread BU = std::thread(&MTMazeStudentSolver::solveWorkerBU, this);

        TD.join();
        BU.join();

        // Fallbacks: reconstruct from route bits
        if(solution == nullptr || (solution != nullptr && solution->empty()))
        {
            unsigned int endCell = pMaze->getCell(pMaze->getEnd());
            if( (endCell & 0xF0000000) != 0 )
            {
                std::vector<Direction>* soln = new std::vector<Direction>();
                soln->reserve(VECTOR_RESERVE_SIZE);
                Direction from = getDirectionRoute(pMaze->getEnd());
                if(from != Direction::Uninitialized)
                {
                    Choice pChoice = followRoute(soln, pMaze->getEnd(), from);
                    while( !(pChoice.at == pMaze->getStart()) )
                    {
                        pChoice = followRoute(soln, pChoice.at, getDirectionRoute(pChoice.at));
                    }
                    reverseDirections(soln);
                }
                solution = soln;
            }
        }
        return solution;
    }

private:


    // Per-thread visited mask
    unsigned int getVisitedMask(int tid)
    {
        switch(tid & 3)
        {
            case 0: return 0x01000000;
            case 1: return 0x02000000;
            case 2: return 0x04000000;
            default: return 0x08000000;
        }
    }

    static unsigned int getVisitedUnionMask()
    {
        // Union of per-thread visited bits
        return 0x0F000000;
    }

    // Fast index calculation for maze cells
    inline unsigned int cellIndex(Position pos) const
    {
        return (unsigned int)(pos.row * pMaze->width + pos.col);
    }


    inline int tryVisitDual(Position pos, unsigned int myMask, unsigned int otherMask)
    {
        unsigned int idx = cellIndex(pos);
        unsigned int prev = pMaze->poMazeData[idx].fetch_or(myMask, std::memory_order_relaxed);
        if((prev & otherMask) != 0) return 2;
        if((prev & myMask) != 0) return 0;
        if((prev & getVisitedUnionMask()) == 0) return 1;
        return 0;
    }

    // Route marking and reconstruction helpers (mirrored from BFS)
    void setDirectionRoute( Position at, Direction from )
    {
        unsigned int val = pMaze->getCell( at );
        int dir = 0;
        switch(from)
        {
            case Direction::East:  dir = 0x10000000; break;
            case Direction::West:  dir = 0x20000000; break;
            case Direction::South: dir = 0x40000000; break;
            case Direction::North: dir = 0x80000000; break;
            case Direction::Uninitialized: return; 
            default: assert(0); break;
        }
        val |= dir;
        pMaze->setCell(at, val);
    }

    void setDirectionRouteBack( Position at, Direction from )
    {
        unsigned int val = pMaze->getCell( at );
        int dir = 0;
        switch(from)
        {
            case Direction::East:  dir = 0x00100000; break;
            case Direction::West:  dir = 0x00200000; break;
            case Direction::South: dir = 0x00400000; break;
            case Direction::North: dir = 0x00800000; break;
            case Direction::Uninitialized: return; 
            default: assert(0); break;
        }
        val |= dir;
        pMaze->setCell(at, val);
    }

    Direction getDirectionRoute( Position at )
    {
        unsigned int val = pMaze->getCell( at );
        val &= 0xF0000000;
        switch(val)
        {
            case 0x10000000: return Direction::East;
            case 0x20000000: return Direction::West;
            case 0x40000000: return Direction::South;
            case 0x80000000: return Direction::North;
            default: assert(0); return Direction::Uninitialized;
        }
    }

    Direction getDirectionRouteBack( Position at )
    {
        unsigned int val = pMaze->getCell( at );
        val &= 0x00F00000;
        switch(val)
        {
            case 0x00100000: return Direction::East;
            case 0x00200000: return Direction::West;
            case 0x00400000: return Direction::South;
            case 0x00800000: return Direction::North;
            default: assert(0); return Direction::Uninitialized;
        }
    }

    static void reverseDirections(std::vector<Direction> *soln)
    {
        if(soln == nullptr) return;
        size_t n = soln->size();
        if(n < 2) return;
        size_t i = 0;
        size_t j = n - 1;
        while(i < j)
        {
            Direction tmp = (*soln)[i];
            (*soln)[i] = (*soln)[j];
            (*soln)[j] = tmp;
            ++i;
            --j;
        }
    }

    Choice followRoute( std::vector<Direction> *soln, Position at, Direction dir )
    {
        ListDirection Choices;
        Direction go_to = dir;
        Direction came_from = reverseDir(dir);
        at = at.move(go_to);

        do
        {
            if( at == (pMaze->getStart()) )
            {
                soln->push_back(came_from);
                Choice pRet( at, came_from, Choices );
                return pRet;
            }

            Choices = pMaze->getMoves(at);
            Choices.remove(came_from);

            soln->push_back(came_from);

            if(Choices.size() == 1)
            {
                go_to = Choices.begin();
                at = at.move(go_to);
                came_from = reverseDir(go_to);
            }

        } while( Choices.size() == 1 );

        Choice pRet( at, came_from, Choices );
        return pRet;
    }

    std::vector<Direction> *FindRoute(SolutionFoundSkip e)
    {
        std::vector<Direction> *soln = new std::vector<Direction>();
        soln->reserve(VECTOR_RESERVE_SIZE);

        Choice pChoice = followRoute(soln, e.pos, e.from);
        while( !(pChoice.at == pMaze->getStart()) )
        {
            pChoice = followRoute( soln, pChoice.at, getDirectionRoute(pChoice.at) );
        }
        reverseDirections(soln);
        return soln;
    }

    Choice followRouteBack( std::vector<Direction> *soln, Position at, Direction dir )
    {
        ListDirection Choices;
        Direction go_to = dir;
        Direction came_from = reverseDir(dir);
        at = at.move(go_to);

        do
        {
            if( at == (pMaze->getEnd()) )
            {
                soln->push_back(go_to);
                Choice pRet( at, came_from, Choices );
                return pRet;
            }

            Choices = pMaze->getMoves(at);
            Choices.remove(came_from);

            soln->push_back(go_to);

            if(Choices.size() == 1)
            {
                go_to = Choices.begin();
                at = at.move(go_to);
                came_from = reverseDir(go_to);
            }

        } while( Choices.size() == 1 );

        Choice pRet( at, came_from, Choices );
        return pRet;
    }

    std::vector<Direction>* CombineRoutesFromMeet(Position meet)
    {
        std::vector<Direction>* soln = new std::vector<Direction>();
        soln->reserve(VECTOR_RESERVE_SIZE);

        if(!(meet == pMaze->getStart()))
        {
            std::vector<Direction> prefix;
            prefix.reserve(VECTOR_RESERVE_SIZE);
            Direction d0 = getDirectionRoute(meet);
            Choice c = followRoute(&prefix, meet, d0);
            while(!(c.at == pMaze->getStart()))
            {
                c = followRoute(&prefix, c.at, getDirectionRoute(c.at));
            }
            reverseDirections(&prefix);
            for(auto &d : prefix) soln->push_back(d);
        }

        if(!(meet == pMaze->getEnd()))
        {
            Direction d1 = getDirectionRouteBack(meet);
            Choice c2 = followRouteBack(soln, meet, d1);
            while(!(c2.at == pMaze->getEnd()))
            {
                c2 = followRouteBack(soln, c2.at, getDirectionRouteBack(c2.at));
            }
        }
        return soln;
    }

    void solveWorkerTD()
    {
        const unsigned int myMask = 0x01000000;
        const unsigned int otherMask = 0x02000000;
        const int STACK_CAP = VECTOR_RESERVE_SIZE;
        Choice* stack = new Choice[STACK_CAP];
        int sp = 0;
        int iter = 0;
        try
        {
            Choice first = firstChoice(pMaze->getStart());
            if(sp < STACK_CAP) stack[sp++] = first;
            while(true)
            {
                if(sp == 0) break;
                if((iter++ & 31) == 0) { if(found.load(std::memory_order_relaxed)) break; }
                Choice ch = stack[--sp];
                int vr = tryVisitDual(ch.at, myMask, otherMask);
                if(vr == 0)
                {
                    continue;
                }
                if(vr == 2)
                {
                    if(ch.from != Direction::Uninitialized)
                    {
                        setDirectionRoute(ch.at, ch.from);
                    }
                    if(found.exchange(true, std::memory_order_acq_rel))
                    {
                        delete[] stack;
                        return;
                    }
                    std::vector<Direction>* soln = CombineRoutesFromMeet(ch.at);
                    solution = soln;
                    delete[] stack;
                    return;
                }
                if(ch.from != Direction::Uninitialized)
                {
                    setDirectionRoute(ch.at, ch.from);
                }
                auto tryPushDir = [&](Direction dir){
                    try
                    {
                        Choice nxt = follow(ch.at, dir);
                        if(sp < STACK_CAP) stack[sp++] = nxt;
                    }
                    catch (SolutionFoundSkip e)
                    {
                        if(found.exchange(true, std::memory_order_acq_rel))
                        {
                            delete[] stack;
                            return true;
                        }
                        setDirectionRoute(pMaze->getEnd(), e.from);
                        std::vector<Direction>* soln = FindRoute(e);
                        solution = soln;
                        delete[] stack;
                        return true;
                    }
                    return false;
                };
                Direction base[4] = { Direction::South, Direction::East, Direction::West, Direction::North };
                for(int k=0; k<4; ++k)
                {
                    Direction d = base[k];
                    if(d == Direction::South && ch.pChoices.south == Direction::South) { if(tryPushDir(Direction::South)) return; }
                    if(d == Direction::East  && ch.pChoices.east  == Direction::East ) { if(tryPushDir(Direction::East )) return; }
                    if(d == Direction::West  && ch.pChoices.west  == Direction::West ) { if(tryPushDir(Direction::West )) return; }
                    if(d == Direction::North && ch.pChoices.north == Direction::North) { if(tryPushDir(Direction::North)) return; }
                }
            }
            delete[] stack;
            return;
        }
        catch (SolutionFoundSkip e)
        {
            if(found.exchange(true, std::memory_order_seq_cst))
            {
                delete[] stack;
                return;
            }
            setDirectionRoute(pMaze->getEnd(), e.from);
            std::vector<Direction>* soln = FindRoute(e);
            solution = soln;
            delete[] stack;
            return;
        }
    }

    void solveWorkerBU()
    {
        const unsigned int myMask = 0x02000000;
        const unsigned int otherMask = 0x01000000;
        const int STACK_CAP = VECTOR_RESERVE_SIZE;
        Choice* stack = new Choice[STACK_CAP];
        int sp = 0;
        int iter = 0;
        try
        {
            Choice first = firstChoice(pMaze->getEnd());
            if(sp < STACK_CAP) stack[sp++] = first;
            while(true)
            {
                if(sp == 0) break;
                if((iter++ & 31) == 0) { if(found.load(std::memory_order_relaxed)) break; }
                Choice ch = stack[--sp];
                int vr = tryVisitDual(ch.at, myMask, otherMask);
                if(vr == 0)
                {
                    continue;
                }
                if(vr == 2)
                {
                    if(ch.from != Direction::Uninitialized)
                    {
                        setDirectionRouteBack(ch.at, ch.from);
                    }
                    if(found.exchange(true, std::memory_order_acq_rel))
                    {
                        delete[] stack;
                        return;
                    }
                    std::vector<Direction>* soln = CombineRoutesFromMeet(ch.at);
                    solution = soln;
                    delete[] stack;
                    return;
                }
                if(ch.from != Direction::Uninitialized)
                {
                    setDirectionRouteBack(ch.at, ch.from);
                }
                auto tryPushDir = [&](Direction dir){
                    try
                    {
                        Choice nxt = follow(ch.at, dir);
                        if(sp < STACK_CAP) stack[sp++] = nxt;
                    }
                    catch (SolutionFoundSkip e)
                    {
                        if(found.exchange(true, std::memory_order_acq_rel))
                        {
                            delete[] stack;
                            return true;
                        }
                        setDirectionRouteBack(e.pos, e.from);
                        std::vector<Direction>* soln = CombineRoutesFromMeet(e.pos);
                        solution = soln;
                        delete[] stack;
                        return true;
                    }
                    return false;
                };
                Direction base[4] = { Direction::North, Direction::West, Direction::East, Direction::South };
                for(int k=0; k<4; ++k)
                {
                    Direction d = base[k];
                    if(d == Direction::North && ch.pChoices.north == Direction::North) { if(tryPushDir(Direction::North)) return; }
                    if(d == Direction::West  && ch.pChoices.west  == Direction::West ) { if(tryPushDir(Direction::West )) return; }
                    if(d == Direction::East  && ch.pChoices.east  == Direction::East ) { if(tryPushDir(Direction::East )) return; }
                    if(d == Direction::South && ch.pChoices.south == Direction::South) { if(tryPushDir(Direction::South)) return; }
                }
            }
            delete[] stack;
            return;
        }
        catch (SolutionFoundSkip e)
        {
            if(found.exchange(true, std::memory_order_seq_cst))
            {
                delete[] stack;
                return;
            }
            setDirectionRouteBack(e.pos, e.from);
            std::vector<Direction>* soln = CombineRoutesFromMeet(e.pos);
            solution = soln;
            delete[] stack;
            return;
        }
    }
};

#endif

// --- End of File ----
