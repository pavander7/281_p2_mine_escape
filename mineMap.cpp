// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F

#include "mineMap.h"
#include <cassert>

using namespace std;

Mine::Mine (bool v_in, bool s_in, bool m_in, uint32_t N_in) : v(v_in), s(s_in), m(m_in), N(N_in) {
    char mode;
    cin >> mode;
    //cout << "mode: " << mode << endl;

    numTiles = 0;
    numRubble = 0;
    string junk;
    uint16_t sRow = 0;
    uint16_t sCol = 0;
    Tile temp2 = {0,0,0};
    spawn = &temp2;

    cin >> junk >> this->size >> junk >> sRow >> sCol;
    this->spawn->row = sRow;
    this->spawn->col = sCol;
    grid.reserve(size);
    clearGrid.reserve(size);
    
    //cout << junk << endl;
    
    stringstream ss;
    uint32_t seed = 0;
    uint32_t max_rubble = 0;
    uint32_t tnt = 0;
    switch (mode) {
        case 'M':
            //nothing to do here lol
            break;
        case 'R':
            cin >> junk >> seed >> junk >> max_rubble >> junk >> tnt;
            P2random::PR_init(ss, size, seed, max_rubble, tnt);
            break;
        default:
            assert(false);
            break;
    }

    istream &in = (mode == 'M') ? cin : ss;

    int val = 0;
    for (uint16_t r = 0; r < this->size; r++) {
        //cout << "[";
        vector<Tile*> line;
        vector<bool> clearLine;
        vector<bool> tntLine;
        line.reserve(size);
        clearLine.reserve(size);
        tntLine.reserve(size);
        for (uint16_t c = 0; c < this->size; c++) {
            in >> val;
            /*cout << val << " ";
            if (val < 100) cout << " ";
            if (val < 10) cout << " "; */
            Tile* here = new Tile();
            *here = {r, c, val};
            clearLine.push_back(false);
            tntLine.push_back(false);
            line.push_back(here);
        }
        //cout << "] " << r << "\n";
        grid.push_back(line);
        clearGrid.push_back(clearLine);
        tntGrid.push_back(tntLine);
    } spawn = grid[sRow][sCol];
    
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

bool easyComp::operator() (const Tile a, const Tile b) const {
    if (a.rubble == b.rubble) {
        if (a.col == b.col) {
            return (a.row < b.row);
        } else return (a.col < b.col);
    } else return a.rubble < b.rubble;
}

bool hardComp::operator() (const Tile a, const Tile b) const {
    if (a.rubble == b.rubble) {
        if (a.col == b.col) {
            return (a.row > b.row);
        } else return (a.col > b.col);
    } else return a.rubble > b.rubble;
}

uint8_t Mine::investigate() {
    //cout << "pq size: " << pq.size() << endl;
    if (pq.top() == nullptr) {
        pq.pop();
        cerr << "falsepop\n";
    } else if (pq.top()->rubble == -1) {
        Tile* temp = pq.top();
        pq.pop();
        explode(temp);
        //delete temp;
    } else {
        Tile* temp = pq.top();
        uint16_t r = pq.top()->row;
        uint16_t c = pq.top()->col;
        uint8_t result = 0;
        // can assume non-edge (edge tile should trigger win condition)
        
        //cout << "investigate: [" << r << "," << c << "] r:" << temp->rubble << endl;

        pq.pop();
        if (s) {
            stats(temp);
        }

        if (temp->rubble > 0) {
            if (v) cout << "Cleared: " << temp->rubble << " at [" << temp->row << "," << temp->col << "]\n";
            numRubble+= uint32_t(temp->rubble);
            numTiles++;
            if (m) {
                medOut(temp->rubble);
            }
        } //else cout << "discard: [" << r << "," << c << "]\n";
        temp->rubble = 0;
        if (r >= size || c >= size) {
            cout << "caught: (" << r << ", " << c << ")\n";
            assert(false);
        }

        //delete temp;

        if ((r < size && c < size) && (r != 0 && c != 0)) {
            //cout << "surround: [" << r << "," << c << "]\n";
            if(!clearGrid[r+1][c]) discover(grid[r+1][c]);
            if(!clearGrid[r-1][c]) discover(grid[r-1][c]);
            if(!clearGrid[r][c+1]) discover(grid[r][c+1]);
            if(!clearGrid[r][c-1]) discover(grid[r][c-1]);
        } else {
            result = 1;
            //cout << "won at [" << r << "," << c << "]\n";
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
    clearGrid[place->row][place->col] = true;
}

void Mine::explode(Tile* place) {
    priority_queue<Tile*, vector<Tile*>, tileComp> TNTq;
    size_t r = place->row;
    size_t c = place->col;

    TNTq.push(grid[r][c]);
    tntGrid[r][c] = true;
    //int round = 0;

    while (!TNTq.empty()) {
        //cout << "round: " << round++;
        //cerr << "TNTland\n";
        
        r = TNTq.top()->row;
        c = TNTq.top()->col;
        Tile* temp = TNTq.top();
        //cout << " [" << r << "," << c << "]: " << temp->rubble << endl;

        if (TNTq.top()->rubble == -1) {
            //cout << "boom\n";
            clearGrid[r][c] = true;
            tntGrid[r][c] = true;

            TNTq.pop();
            if (r < size) { 
                if(!tntGrid[r+1][c] && grid[r+1][c]->rubble != 0) {
                    TNTq.push(grid[r+1][c]);
                    clearGrid[r+1][c] = true;
                    tntGrid[r+1][c] = true;
                    //cout << "tntfind d [" << r+1 << "," << c << "]\n";
                } 
            } if (r != 0) { 
                if(!tntGrid[r-1][c] && grid[r-1][c]->rubble != 0) {
                    TNTq.push(grid[r-1][c]);
                    clearGrid[r-1][c] = true;
                    tntGrid[r-1][c] = true;
                    //cout << "tntfind u [" << r-1 << "," << c << "]\n";
                } 
            } if (c < size) { 
                if(!tntGrid[r][c+1] && grid[r][c+1]->rubble != 0) {
                    TNTq.push(grid[r][c+1]);
                    clearGrid[r][c+1] = true;
                    tntGrid[r][c+1] = true;
                    //cout << "tntfind r [" << r << "," << c+1 << "]\n";
                }
            } if (c != 0) { 
                if(!tntGrid[r][c-1] && grid[r][c-1]->rubble != 0) { 
                    TNTq.push(grid[r][c-1]);
                    clearGrid[r][c-1] = true;
                    tntGrid[r][c-1] = true;
                    //cout << "tntfind l [" << r << "," << c-1 << "]\n";
                } 
            } 
            if (v) cout << "TNT explosion at [" << temp->row << "," << temp->col << "]!\n";
            if (s) {
                stats(temp);
            }

            temp->rubble = 0;
            //delete temp;
        } else {
            if (v) cout << "Cleared by TNT: " << temp->rubble << " at [" << temp->row << "," << temp->col << "]\n";
            if (temp->rubble > 0) {
                numRubble+= uint32_t(temp->rubble);
                numTiles++;
                if (m) {
                    medOut(temp->rubble);                   
                } 
            } if (s) {
                stats(temp);
            } 
            TNTq.pop();
            //cout << "x";
            temp->rubble = 0;
            discover(temp);
            //fix invariants here
        }
    }
}

bool Mine::lost() {
    return pq.empty();
}

void Mine::statsOut() {
    cout << "First tiles cleared:\n";
    for(uint16_t n = 0; n < min(N,uint32_t(firstCleared.size())); n++) {
        Tile* f = &firstCleared[n];
        if (f->rubble == -1) cout << "TNT";
        else cout << f->rubble;
        cout << " at [" << f->row << "," << f->col << "]\n";
    }
    cout << "Last tiles cleared:\n";
    for(uint16_t n = 0; n < min(N,uint32_t(lastCleared.size())); n++) {
        Tile* l = &lastCleared[n];
        if (l->rubble == -1) cout << "TNT";
        else cout << l->rubble;
        cout << " at [" << l->row << "," << l->col << "]\n";
    }
    cout << "Easiest tiles cleared:\n";
    //sort(easiest.begin(), easiest.end(), easyComp());
    for(uint16_t n = 0; n < min(N,uint32_t(easiest.size())); n++) {
        Tile* e = &easiest[n];
        if (e->rubble == -1) cout << "TNT";
        else cout << e->rubble;
        cout << " at [" << e->row << "," << e->col << "]\n";
    }
    cout << "Hardest tiles cleared:\n";
    //sort(hardest.begin(), hardest.end(), hardComp());
    for(uint16_t n = 0; n < min(N,uint32_t(hardest.size())); n++) {
        Tile* h = &hardest[n];
        if (h->rubble == -1) cout << "TNT";
        else cout << h->rubble;
        cout << " at [" << h->row << "," << h->col << "]\n";
    }
}

void Mine::manualClear() {
    for (uint16_t r = 0; r < size; r++) {
        for (uint16_t c = 0; c < size; c++) {
            if (grid[r][c] != nullptr) {
                delete grid[r][c];
            } else cout << "falsepop\n";
        }
    }
}

void sort_insert(deque<int> &book, int elt) {
    deque<int> side;
    if(book.empty()) {
        book.push_back(elt);
        //cout << "book: " << elt << endl;
        return;
    } else {
        while (!book.empty()) {
            if (book.back() < elt) {
                side.push_back(book.back());
                book.pop_back();
            } else break;
        } book.push_back(elt);
        while (!side.empty()) {
            book.push_back(side.back());
            side.pop_back();
        }
    }
    /*cout << "book: ";
    for (size_t q = 0; q < book.size(); q++) {
        cout << book[q] << " ";
    } cout << endl;*/
}

void sort_insert(vector<Tile> &book, Tile elt, bool easy) {
    deque<Tile> side;
    if(book.empty()) {
        book.push_back(elt);
        //cout << "book: " << elt << endl;
        return;
    } else {
        if (easy) {
            easyComp cmp;
            while (!book.empty()) {
                if (!cmp(book.back(),elt)) {
                    side.push_back(book.back());
                    book.pop_back();
                } else break;
            } book.push_back(elt);
            while (!side.empty()) {
                book.push_back(side.back());
                side.pop_back();
            }
        } else {
            hardComp cmp;
            while (!book.empty()) {
                if (!cmp(book.back(),elt)) {
                    side.push_back(book.back());
                    book.pop_back();
                } else break;
            } book.push_back(elt);
            while (!side.empty()) {
                book.push_back(side.back());
                side.pop_back();
            }
        }
    }
    /*cout << "book: ";
    for (size_t q = 0; q < book.size(); q++) {
        cout << book[q] << " ";
    } cout << endl;*/
}

void Mine::medOut (int elt) {
    sort_insert(medVec, elt);
    cout << "Median difficulty of clearing rubble is: ";
    if (medVec.size() == 1) {
        cout << float(medVec[0]);
    } else if (medVec.size()%2 == 0) {
        cout << (float(medVec[size_t(medVec.size())/size_t(2)-size_t(1)]) + float(medVec[size_t(medVec.size())/size_t(2)]))/float(2);
    } else { 
        cout << float(medVec[(medVec.size()-size_t(1))/size_t(2)]);
    }
    cout << endl;
}

void Mine::stats(Tile* elt) {
    if (elt->rubble == 0) return;
    if (firstCleared.size() < N) {
        firstCleared.push_back(*elt);
        lastCleared.push_front(*elt);
    } else {
        if (lastCleared.size () < N) {
            lastCleared.push_front(*elt);
        } else if (lastCleared.size() == N) {
            lastCleared.pop_back();
            lastCleared.push_front(*elt);
        } else {
            cerr << "stats err\n";
            assert(false);
        }
    }
    if (easiest.size() == 0) {
        easiest.push_back(*elt);
    } else if (easiest.size() < N) {
        sort_insert(easiest, *elt, true);
    } else if (easiest.size() == N) {
        easiest.pop_back();
        sort_insert(easiest, *elt, true);
    }
    if (hardest.size() == 0) {
        hardest.push_back(*elt);
    } else if (hardest.size() < N) {
        sort_insert(hardest, *elt, false);
    } else if (hardest.size() == N) {
        hardest.pop_back();
        sort_insert(hardest, *elt, false);
    } return;
}