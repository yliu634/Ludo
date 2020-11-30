#include "common.h"
#include "cstdlib"
#include "Othello/data_plane_othello.h"
#include "MinimalPerfectCuckoo/minimal_perfect_cuckoo.h"
#include "DPH/dph.h"

using namespace std;

uint64_t usernum = 10;
uint32_t dcnum = 5;
double mobileuserrate = 0.2;

vector<vector<int>> dijkstraDC;//dc distance form.
map<uint64_t, uint16_t> mobileuserlist;
//map<ID_Digest, DC> mobileuser;

template<class ID_Digest, class DC>
struct user {
    uint32_t id;
    DC HomeLoc;
    DC VisitorLoc;
    //uint16_t digest = H(id) & (uint(-1) >> (32 - ID_Digest_len));
    double HomeInfoRatio = 0.5;
};

template<class DC>
struct Datacenter {
    DC DCindex;
    unsigned usernumber = 0;
    double OutLocInfoRatio = 0.5;
};


template<class ID_Digest, class DC>
void testLudoHLR(vector<user<ID_Digest, DC>> IU) {
//vector<ID_Digest> &keys, vector<DC> &values, uint64_t nn, vector<DC> &zipfianKeys){
//mpc.h 1005 line tell you what you found.
//in order to call cp(nn), mobile-list -> &keys, &values, nn;
    unsigned nn = mobileuserrate * usernum;
    //construct cp, 3th parameter is VL = uint16_t, 200~300 DCs, I hope it could be 8-length.
    ControlPlaneMinimalPerfectCuckoo<ID_Digest, DC, 8> cp(nn);
    for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.cend(); iter++) {
        cp.insert(iter->first, iter->second);
    }
    cp.prepareToExport();
    //same as above with cp(nn)
    DataPlaneMinimalPerfectCuckoo<ID_Digest, DC, 8> dp(cp);

    for (unsigned iDCcnt = 1; iDCcnt <= dcnum; ++iDCcnt) {
        unsigned Req = 10, cost = 0, iReq = 1;
        for (iReq = 1; iReq <= 0.5 * Req; ++iReq) {
            auto tmp = IU[rand() % usernum];//H
            DC Home = tmp.HomeLoc;
            DC LudoFindVisitor;
            if (!dp.lookUp(Home, LudoFindVisitor)) {
                cout << "Not found in Mobile user list" << endl;
                //cost += 2 * dijkstra[iDCcnt][tmp.H];
            } else {
                //cost += (dijks[iDCcnt][tmp.H]>dijks[iDCcnt][LudoFindVisitor])?
                // 2 * dijkstra[iDCcnt][LudoFindVisitor]: 2 * dijkstra[iDCcnt][LudoFindVisitor];
                /* uint8_t Hflag = ((rand() % 100) <= 100 * tmp.HomeInfoRatio) ? 1 : 0;//info in home.
                uint8_t Hnear = dijks[iDCcnt][tmp.H]>dijks[iDCcnt][LudoFindVisitor]? 1:0;
                if(Hflag == 1 && Hnear==1)
                      cost += 2*(dijkstra[iDCcnt][tmp.H];
                else if(Hflag == 0 && Hnear==0)
                    cost += 2*(dijkstra[iDCcnt][LudoFindVisitor];
                else
                    cost +=2(dijkstra[iDCcnt][LudoFindVisitor]+dijkstra[iDCcnt][LudoFindVisitor]);
                */
            }
        }
    }
}

template<class ID_Digest, class DC>
void testStateofArt(vector<user<ID_Digest, DC>> IU) {
    for (unsigned iDCcnt = 1; iDCcnt <= dcnum; ++iDCcnt) {
        //gen_request for every DC, OutLocInfoRatio percent:-> not this DC;
        //1-ratio percent:-> this DC; Total 1000 times; 0.5 means outLocInfoRatio;
        unsigned Req = 10, cost = 0, iReq = 1;
        for (iReq = 1; iReq <= 0.5 * Req; ++iReq) {
            auto tmp = IU[rand() % usernum];//H
            DC Home = tmp.HomeLoc;
            // not in home prob.
            if (rand() % 100 > tmp.HomeInfoRatio) {
                auto Visitor = mobileuserlist.find(tmp.id);
                if (Visitor == mobileuserlist.end()) {
                    cout << "Not found in Mobile List" << endl;
                } else {
                    DC VisitPlace = Visitor->second;
                    //calculate the dijsktra[finded V][vector[DCcnt]]
                    //cost +=
                }
            }
            //cost +=dijsktra[tmp.H][vector[DCnt]];
        }
    }
    //cost for unique DC;
}


template<class DC>
void gen_dijkstraDC() {

}

template<class ID_Digest, class DC>
void testHLR() {
    //gen_dijkstraDC();

    LFSRGen<uint32_t> keyGen(0x1234567801234567ULL, usernum, 0);
    vector<user<ID_Digest, DC>> IU;

    for (unsigned i = 1; i <= usernum; i++) {
        user<ID_Digest, DC> tmp;
        keyGen.gen(&tmp.id);
        tmp.HomeLoc = rand() % dcnum;
        tmp.HomeInfoRatio = 0.5;
        if (i <= mobileuserrate * usernum) {
            tmp.VisitorLoc = rand() % dcnum;//mobile user list gen.
            mobileuserlist.insert(pair<ID_Digest, DC>(tmp.id, tmp.VisitorLoc));
        } else
            tmp.VisitorLoc = tmp.HomeLoc;
        IU.push_back(tmp);
    }

    //testStateofArt<ID_Digest, DC>(IU);
    testLudoHLR<ID_Digest, DC>(IU);

/*
    for (const auto el: mobileuserlist)
        //cout << el.id<<" "<<el.HomeLoc<<" "<<el.VisitorLoc<<" " << endl;
        cout << el.first << " " << el.second << endl;
*/
}

int main(int argc, char **argv) {
    commonInit();
    testHLR<uint64_t, uint16_t>();
}


