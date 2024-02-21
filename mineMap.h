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

class Mine {
public: 
    Mine(); // default constructor
    Mine(std::istream &in); // M-mode constructor

    bool checkTNT (Tile* place);
    int checkRubble (Tile* place);
    bool checkEdge (Tile* place);
    bool lost();

    friend class tileComp;

    std::uint8_t investigate();
    void discover(Tile* place);
    void explode(Tile* place);

    uint32_t numTiles;
    uint32_t numRubble;

private:
    std::vector<std::vector<Tile*> > grid;
    std::vector<std::vector<bool> > clearGrid;
    uint16_t size;
    Tile* spawn;
    std::priority_queue<Tile*, std::vector<Tile*>, tileComp> pq;
};