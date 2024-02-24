// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F

#include <vector>
#include <queue>
#include <deque>
#include <iostream>
#include "P2random.h"

struct Tile {
    std::uint16_t row;
    std::uint16_t col;
    int rubble;
};

class tileComp {
public:
    bool operator() (const Tile* a, const Tile* b) const;
};

class easyComp {
public:
    bool operator() (const Tile a, const Tile b) const;
};

class hardComp {
public:
    bool operator() (const Tile a, const Tile b) const;
};

class Mine {
public: 
    Mine( bool v_in, bool s_in, bool m_in, uint32_t N); // M-mode constructor

    bool checkTNT (Tile* place);
    int checkRubble (Tile* place);
    bool checkEdge (Tile* place);
    bool lost();

    friend class tileComp;
    friend class easyComp;
    friend class hardComp;

    std::uint8_t investigate();
    void discover(Tile* place);
    void explode(Tile* place);

    void statsOut();
    void medOut(int elt);
    void manualClear();

    uint32_t numTiles, numRubble;

private:
    std::vector<std::vector<Tile*> > grid;
    std::vector<std::vector<bool> > findGrid, clearGrid, tntGrid;
    std::uint32_t size;
    Tile* spawn;
    std::priority_queue<Tile*, std::vector<Tile*>, tileComp> pq;
    bool v, s, m;
    std::uint32_t N;
    std::deque<Tile> firstCleared, lastCleared;
    std::vector<Tile> easiest;
    std::vector<Tile> hardest;
    std::priority_queue<int, std::vector<int>, std::less<int>> leftMed;
    std::priority_queue<int, std::vector<int>, std::greater<int>> rightMed;
    float median;
    void stats(Tile* elt);
};

void sort_insert(std::deque<int> &book, int elt);
void sort_insert(std::vector<Tile> &book, Tile elt, bool easy);