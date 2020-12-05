#include "common.h"
#include "cstdlib"
#include "Othello/data_plane_othello.h"
#include "MinimalPerfectCuckoo/minimal_perfect_cuckoo.h"
#include "dijsktraform.h"
#include "home_visit.h"
//#include <fstream>

using namespace std;

template<class ID, class DC>
void testHLR() {
    LudoNearStateofArt<ID, DC, 1, 16000> dd;
    dd.test();
}

int main(int argc, char **argv) {
    commonInit();
    testHLR<uint32_t, uint8_t>();

}


