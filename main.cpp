#include "common.h"
#include "cstdlib"
#include "Othello/data_plane_othello.h"
#include "MinimalPerfectCuckoo/minimal_perfect_cuckoo.h"
#include "dijsktraform.h"
#include "home_visit.h"

using namespace std;

template<class ID, class DC>
void testHLR(uint8_t &ty, ID &T) {
    LudoNearStateofArt<ID, DC> dd(ty, T);
    dd.test();
}

int main(int argc, char **argv) {
    commonInit();
    uint8_t ty = 2; uint32_t T = 512000;
    //for (int i = 0; i < 6; i++) {
        //T *= 2;
        testHLR<uint32_t, uint8_t>(ty, T);
    //}
}


