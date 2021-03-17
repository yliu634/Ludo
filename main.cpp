#include "common.h"
#include "cstdlib"
#include "Othello/data_plane_othello.h"
#include "MinimalPerfectCuckoo/minimal_perfect_cuckoo.h"
#include "dijsktraform.h"
#include "home_visit.h"

using namespace std;

template<class ID, class DC>
void testHLR(ID &T) {
    LudoNearStateofArt<ID, DC> dd(T);
    dd.test();
}

int main(int argc, char **argv) {

    commonInit();
    uint32_t T = 500;
    for (int i = 0; i < 5; i++) {
        T *= 2;
        testHLR<uint32_t, uint16_t>(T);
    }

}


