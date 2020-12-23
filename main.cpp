#include "common.h"
#include "cstdlib"
#include "Othello/data_plane_othello.h"
#include "MinimalPerfectCuckoo/minimal_perfect_cuckoo.h"
#include "dijsktraform.h"
#include "home_visit.h"
//#include <fstream>

using namespace std;

template<class ID, class DC>
void testHLR(uint8_t &ty, ID &T) {
    LudoNearStateofArt<ID, DC> dd(ty, T);
    dd.test();
}

int main(int argc, char **argv) {
    commonInit();
    uint8_t ty = 1; uint32_t T = 8;
    for (int i = 0; i < 1; i++) {
        T *= 2;
        testHLR<uint32_t, uint16_t>(ty, T);
    }
}


