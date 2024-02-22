// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F

#include "mineMap.h"
#include <cassert>

using namespace std;

Mine::Mine (std::istream &in, bool v_in, bool s_in, bool m_in) : v(v_in), s(s_in), m(m_in) {
    //cout << "checkpoint 3.1: entered constructor\n";
    numTiles = 0;
    numRubble = 0;
    string junk;
    uint16_t sRow = 0;
    uint16_t sCol = 0;
    Tile temp2 = {0,0,0};
    spawn = &temp2;

    in >> junk >> this->size >> junk >> sRow >> sCol;
    this->spawn->row = sRow;
    this->spawn->col = sCol;
    grid.reserve(size);
    clearGrid.reserve(size);
    //cout << junk << " " << this->size << " " << sRow << " " << sCol << endl;
    //cout << "checkpoint 3.2: prelim vals entered\n";
    int temp = 0;
    for (uint16_t r = 0; r < this->size; r++) {
        //cout << "[";
        vector<Tile*> line;
        vector<bool> clearLine;
        line.reserve(size);
        clearLine.reserve(size);
        for (uint16_t c = 0; c < this->size; c++) {
            in >> temp;
            /*cout << temp << " ";
            if (temp < 100) cout << " ";
            if (temp < 10) cout << " "; */
            Tile* here = new Tile();
            *here = {r, c, temp};
            clearLine.push_back(false);
            line.push_back(here);
        }
        //cout << "] " << r << "\n";
        grid.push_back(line);
        clearGrid.push_back(clearLine);
    } spawn = grid[sRow][sCol];
    //cout << spawn->row << " " << spawn->col << endl;

    //cout << "checkpoint 3.3: map filled\n";
    discover(spawn);
}

bool Mine::checkTNT(Tile* place) {
    return (place->rubble == -1);
}

int Mine::checkRubble(Tile* place) {
    if (place->rubble == -1) {
        return 0;
    }
    return place->rubble;
}

bool tileComp::operator() (const Tile* a, const Tile* b) const {
    if (a->rubble == -1 || b->rubble == -1) {
        if (a->rubble == b->rubble) {
            return (a->col > b->col);
        } else return (a->rubble > b->rubble);
    } else if (a->rubble == b->rubble) {
        if (a->col == b->col) {
            return (a->row > b->row);
        } else return (a->col > b->col);
    } else return a->rubble > b->rubble;
}

uint8_t Mine::investigate() {
    if (pq.top() == nullptr) {
        pq.pop();
        cerr << "falsepop\n";
    } else {
        Tile* temp = pq.top();
        uint16_t r = pq.top()->row;
        uint16_t c = pq.top()->col;
        uint8_t result = 0;
        // can assume non-edge (edge tile should trigger win condition)
        /*
        if (r == 0 || c == 0) {
            cout << "BACKEDGE\n";
        } */

        if (temp->rubble > 0) {
            if (v) cout << "Cleared: " << temp->rubble << " at [" << temp->row << "," << temp->col << "]\n";
            numRubble+= uint32_t(temp->rubble);
            numTiles++;
        }
        temp->rubble = 0;
        if (r >= size || c >= size) {
            cout << "caught: (" << r << ", " << c << ")\n";
            assert(false);
        }
        clearGrid[r][c] = true;
        pq.pop();
        delete temp;

        /*if (r == size - 1 || c == size - 1 || r == 0 || c == 0) {
            cerr << "FOUND IT\n";
            assert(false);
        }*/
        if ((r < size && c < size) && (r != 0 && c != 0)) {
            if(!clearGrid[r+1][c]) discover(grid[r+1][c]);
            if(!clearGrid[r-1][c]) discover(grid[r-1][c]);
            if(!clearGrid[r][c+1]) discover(grid[r][c+1]);
            if(!clearGrid[r][c-1]) discover(grid[r][c-1]);
        } else {
            result = 1;
        }

        return result;
    } return 0; // for compiler, shouldn't ever end up here
    /*Check for tnt
    if (grid[r+1][c+1]->rubble == -1) explode(place);
    else if (grid[r+1][c-1]->rubble == -1) explode(place);
    else if (grid[r-1][c-1]->rubble == -1) explode(place);
    else if (grid[r-1][c+1]->rubble == -1) explode(place);
    else {
    } */
}

void Mine::discover(Tile* place) {
    //cout << "discovered: (" << place->row << ", " << place->col << ") rubble: " << place->rubble << endl;
    pq.push(place);
}

void Mine::explode(Tile* place) {
    priority_queue<Tile*, vector<Tile*>, tileComp> TNTq;
    size_t r = place->row;
    size_t c = place->col;

    TNTq.push(grid[r][c]);

    while (!TNTq.empty()) {
        if (TNTq.top()->rubble == -1) {
            size_t r2 = TNTq.top()->row;
            size_t c2 = TNTq.top()->col;

            if(!clearGrid[r2+1][c2]) TNTq.push(grid[r2+1][c2]);
            if(!clearGrid[r2-1][c2]) TNTq.push(grid[r2-1][c2]);
            if(!clearGrid[r2][c2+1]) TNTq.push(grid[r2][c2+1]);
            if(!clearGrid[r2][c2-1]) TNTq.push(grid[r2][c2-1]);

            Tile* temp = TNTq.top();
            if (v) cout << "TNT explosion at [" << temp->row << "," << temp->col << "]/n";
            temp->rubble = 0;
            clearGrid[r][c] = true;
            TNTq.pop();
            delete temp;
        } else {
            Tile* temp = TNTq.top();
            if (v) cout << "Cleared by TNT: " << temp->rubble << " at [" << temp->row << "," << temp->col << "]/n";
            temp->rubble = 0;
            TNTq.pop();
            pq.push(temp);
            //fix invariants here
        }
    }
}

bool Mine::lost() {
    return pq.empty();
}