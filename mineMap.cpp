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
    findGrid.reserve(size);
    clearGrid.reserve(size);
    tntGrid.reserve(size);
    
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
    vector<bool> boolLine;
    boolLine.reserve(size);
    for (uint16_t r = 0; r < this->size; r++) {
        //cout << "[";
        vector<Tile*> line;
        line.reserve(size);
        for (uint16_t c = 0; c < this->size; c++) {
            in >> val;
            /*cout << val << " ";
            if (val < 100) cout << " ";
            if (val < 10) cout << " "; */
            Tile* here = new Tile();
            *here = {r, c, val};
            line.push_back(here);
        }
        boolLine.push_back(false);
        //cout << "] " << r << "\n";
        grid.push_back(line);
    } spawn = grid[sRow][sCol];
    for (uint16_t z = 0; z < this->size; z++) {
        findGrid.push_back(boolLine);
        clearGrid.push_back(boolLine);
        tntGrid.push_back(boolLine);
    }
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
            if (a->col == b->col) {
                return (a->row > b->row);
            } else return (a->col > b->col);
        } else {
            if (a->rubble == -1) return false;
            else return true;
        }
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
    //cout << "pq top [" << pq.top()->row << "," << pq.top()->col << "]: " << pq.top()->rubble << endl;
    uint8_t result = 0;
    Tile* temp = pq.top();
    //findGrid[temp->row][temp->col] = true;
    if (tntGrid[temp->row][temp->col]) {
        pq.pop();
        //cout << "discard: [" << temp->row << "," << temp->col << "]: " << temp->rubble << endl;
    } else if (temp->rubble == -1) {
        pq.pop();
        if (!tntGrid[temp->row][temp->col]) explode(temp);
        //delete temp;
    } else {
        uint16_t r = temp->row;
        uint16_t c = temp->col;
        
        // can assume non-edge (edge tile should trigger win condition)
        
        //cout << "investigate: [" << r << "," << c << "] r:" << temp->rubble << endl;
        pq.pop();
        clearGrid[temp->row][temp->col] = true;
        tntGrid[temp->row][temp->col] = true;

        if (temp->rubble != 0) {
            if (v) cout << "Cleared: " << temp->rubble << " at [" << temp->row << "," << temp->col << "]\n";
            numRubble+= uint32_t(temp->rubble);
            numTiles++;
            if (m) {
                medOut(temp->rubble);
            }
        } //else cout << "discard: [" << r << "," << c << "]\n";
        if (r >= size || c >= size) {
            cout << "caught: (" << r << ", " << c << ")\n";
            assert(false);
        }

        temp->rubble = 0;
        
        if (s) {
            stats(temp);
        }

        //delete temp;
        if ((r < (size - uint32_t(1)) && c < (size - uint32_t(1))) && (r != 0 && c != 0)) {
            //cout << "surround: [" << r << "," << c << "]: " << temp->rubble << endl;
            if(!findGrid[r+1][c]) discover(grid[r+1][c]);
            if(!findGrid[r-1][c]) discover(grid[r-1][c]);
            if(!findGrid[r][c+1]) discover(grid[r][c+1]);
            if(!findGrid[r][c-1]) discover(grid[r][c-1]);
        } else {
            result = 1;
            //cout << "won at [" << r << "," << c << "]\n";
        }

    } 
    return result; 
}

void Mine::discover(Tile* place) {
    //cout << "discovered: [" << place->row << "," << place->col << "] rubble: " << place->rubble << endl;
    pq.push(place);
    findGrid[place->row][place->col] = true;
}

void Mine::explode(Tile* place) {
    priority_queue<Tile*, vector<Tile*>, tileComp> TNTq;
    priority_queue<Tile*, vector<Tile*>, tileComp> Rq;
    uint32_t r = place->row;
    uint32_t c = place->col;

    TNTq.push(grid[r][c]);
    tntGrid[r][c] = true;
    findGrid[r][c] = true;
    //int round = 0;

    while (!TNTq.empty()) {
        r = TNTq.top()->row;
        c = TNTq.top()->col;
        Tile* temp = TNTq.top();

        findGrid[r][c] = true;
        clearGrid[r][c] = true;
        tntGrid[r][c] = true;

        TNTq.pop();
        temp->rubble = 0;

        if (r < size) if (!tntGrid[r+1][c] && grid[r+1][c]->rubble == -1) TNTq.push(grid[r+1][c]);
        if (r != 0) if (!tntGrid[r-1][c] && grid[r-1][c]->rubble == -1) TNTq.push(grid[r-1][c]);
        if (c < size) if (!tntGrid[r][c+1] && grid[r][c+1]->rubble == -1) TNTq.push(grid[r][c+1]);
        if (c != 0) if (!tntGrid[r][c-1] && grid[r][c-1]->rubble == -1) TNTq.push(grid[r][c-1]);

        if (r < size) { 
            if(!tntGrid[r+1][c] && (!clearGrid[r+1][c]) && grid[r+1][c]->rubble > 0) {
                Rq.push(grid[r+1][c]);
                findGrid[r+1][c] = true;
                tntGrid[r+1][c] = true;
                //cout << "tntfind d [" << r+1 << "," << c << "]\n";
            } 
        } if (r != 0) { 
            if(!tntGrid[r-1][c] && (!clearGrid[r-1][c]) && grid[r-1][c]->rubble > 0) {
                Rq.push(grid[r-1][c]);
                findGrid[r-1][c] = true;
                tntGrid[r-1][c] = true;
                //cout << "tntfind u [" << r-1 << "," << c << "]\n";
            } 
        } if (c < size) { 
            if(!tntGrid[r][c+1] && (!clearGrid[r][c+1]) && grid[r][c+1]->rubble > 0) {
                Rq.push(grid[r][c+1]);
                findGrid[r][c+1] = true;
                tntGrid[r][c+1] = true; 
                //cout << "tntfind r [" << r << "," << c+1 << "]\n";
            }
        } if (c != 0) { 
            if(!tntGrid[r][c-1] && (!clearGrid[r][c-1]) && grid[r][c-1]->rubble > 0) { 
                Rq.push(grid[r][c-1]);
                findGrid[r][c-1] = true;
                tntGrid[r][c-1] = true;
                //cout << "tntfind l [" << r << "," << c-1 << "]\n";
            } 
        }

        if (v) cout << "TNT explosion at [" << temp->row << "," << temp->col << "]!\n";
        if (s) {
            stats(temp);
        }
        
    } //cout << "cleanup\n";
    while (!Rq.empty()) {
        r = Rq.top()->row;
        c = Rq.top()->col;
        Tile* temp = Rq.top();

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
        Rq.pop();
        //findGrid[temp->row][temp->col] = true;
        temp->rubble = 0;
        //cout << "x";
        discover(temp);
    }

    /*while (!TNTq.empty()) {
        //cout << "round: " << round++;
        //cerr << "TNTland\n";
        
        r = TNTq.top()->row;
        c = TNTq.top()->col;
        Tile* temp = TNTq.top();
        //cout << " [" << r << "," << c << "]: " << temp->rubble << endl;

        if (TNTq.top()->rubble == -1) {
            //cout << "boom\n";
            findGrid[r][c] = true;
            tntGrid[r][c] = true;
            
            TNTq.pop();
            if (r < size) { 
                if(!tntGrid[r+1][c] && !findGrid[r+1][c] && grid[r+1][c]->rubble != 0) {
                    TNTq.push(grid[r+1][c]);
                    findGrid[r+1][c] = true;
                    tntGrid[r+1][c] = true;
                    //cout << "tntfind d [" << r+1 << "," << c << "]\n";
                } 
            } if (r != 0) { 
                if(!tntGrid[r-1][c] && !findGrid[r-1][c] && grid[r-1][c]->rubble != 0) {
                    TNTq.push(grid[r-1][c]);
                    findGrid[r-1][c] = true;
                    tntGrid[r-1][c] = true;
                    //cout << "tntfind u [" << r-1 << "," << c << "]\n";
                } 
            } if (c < size) { 
                if(!tntGrid[r][c+1] && !findGrid[r][c+1] && grid[r][c+1]->rubble != 0) {
                    TNTq.push(grid[r][c+1]);
                    findGrid[r][c+1] = true;
                    tntGrid[r][c+1] = true;
                    //cout << "tntfind r [" << r << "," << c+1 << "]\n";
                }
            } if (c != 0) { 
                if(!tntGrid[r][c-1] && !findGrid[r][c-1] && grid[r][c-1]->rubble != 0) { 
                    TNTq.push(grid[r][c-1]);
                    //findGrid[r][c-1] = true;
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
            TNTq.pop();
            if (temp->rubble > 0) {
                numRubble+= uint32_t(temp->rubble);
                numTiles++;
                if (m) {
                    medOut(temp->rubble);                   
                }
            } if (s) {
                stats(temp);
            } 
            //cout << "x";
            temp->rubble = 0;
            discover(temp);
            //fix invariants here
        }
    }*/
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
    for(uint16_t n = 0; n < min(N,uint32_t(easiest.size())); n++) {
        Tile* e = &easiest[n];
        if (e->rubble == -1) cout << "TNT";
        else cout << e->rubble;
        cout << " at [" << e->row << "," << e->col << "]\n";
    }
    cout << "Hardest tiles cleared:\n";
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
    //sort_insert(medVec, elt);
    cout << "Median difficulty of clearing rubble is: ";
    if (leftMed.size() == 0 && rightMed.size() == 0) {
        leftMed.push(elt);
        median = elt;
    } else if (leftMed.size() == 1 && rightMed.size() == 0) {
        rightMed.push(elt);
        if(leftMed.top() > rightMed.top()) {
            int leftT = leftMed.top();
            int rightT = rightMed.top();
            leftMed.pop();
            rightMed.pop();
            leftMed.push(rightT);
            rightMed.push(leftT);
        }
        median = float(leftMed.top() + rightMed.top())/float(2.00);
    } else if (leftMed.size() == 0 && rightMed.size() == 1) {
        leftMed.push(elt);
        if(leftMed.top() > rightMed.top()) {
            int leftT = leftMed.top();
            int rightT = rightMed.top();
            leftMed.pop();
            rightMed.pop();
            leftMed.push(rightT);
            rightMed.push(leftT);
        }
    } else {
        if (elt < median) leftMed.push(elt);
        else if (elt > median) rightMed.push(elt);
        else if (leftMed.size() <= rightMed.size()) leftMed.push(elt);
        else rightMed.push(elt);
        if ((leftMed.size() - rightMed.size()) == 2) {
            int temp = leftMed.top();
            leftMed.pop();
            rightMed.push(temp);
            median = float(leftMed.top() + rightMed.top())/float(2.00);
        } else if ((rightMed.size() - leftMed.size()) == 2) {
            int temp = rightMed.top();
            rightMed.pop();
            leftMed.push(temp);
            median = float(leftMed.top() + rightMed.top())/float(2.00);
        } else if ((leftMed.size() - rightMed.size()) == 1) {
            median = leftMed.top();
        } else if ((rightMed.size() - leftMed.size()) == 1) {
            median = rightMed.top();
        } else if (leftMed.size() == rightMed.size()) {
            median = float(leftMed.top() + rightMed.top())/float(2.00);
        } else assert(false);
    }
    /*if (medVec.size() == 1) {
        cout << float(medVec[0]);
    } else if (medVec.size()%2 == 0) {
        cout << (float(medVec[size_t(medVec.size())/size_t(2)-size_t(1)]) + float(medVec[size_t(medVec.size())/size_t(2)]))/float(2);
    } else { 
        cout << float(medVec[(medVec.size()-size_t(1))/size_t(2)]);
    }*/
    cout << median << endl;
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
    easyComp cmp;
    if (easiest.size() == 0) {
        easiest.push_back(*elt);
    } else if (easiest.size() < N) {
        sort_insert(easiest, *elt, true);
    } else if (easiest.size() == N && cmp(*elt,easiest.back())) {
        easiest.pop_back();
        sort_insert(easiest, *elt, true);
    }
    if (hardest.size() == 0) {
        hardest.push_back(*elt);
    } else if (hardest.size() < N) {
        sort_insert(hardest, *elt, false);
    } else if (hardest.size() == N && cmp(easiest.back(),*elt)) {
        hardest.pop_back();
        sort_insert(hardest, *elt, false);
    } return;
}