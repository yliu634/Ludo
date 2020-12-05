#include "common.h"
#include "cstdlib"
#include "Othello/data_plane_othello.h"
#include "MinimalPerfectCuckoo/minimal_perfect_cuckoo.h"
#include "dijsktraform.h"
#include "home_visit.h"
//#include <fstream>

using namespace std;

template<class ID, class DC>
void testHLR(uint8_t &t, uint32_t &T) {
    LudoNearStateofArt<ID, DC> dd(t, T);
    dd.test();
}

int main(int argc, char **argv) {
    commonInit();
    uint8_t t = 2; uint32_t T = 8000;
    for (int i = 0; i < 11; i++) {
        T *= 2;
        testHLR<uint32_t, uint8_t>(t, T);
    }
}


