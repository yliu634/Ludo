#include "common.h"
#include "cstdlib"

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
void testStateofArt(user<ID_Digest, DC> IU) {
    for (unsigned iDCcnt = 1; iDCcnt <= dcnum; ++iDCcnt) {
        //gen_request for every DC, OutLocInfoRatio percent:-> not this DC;
        //1-ratio percent:-> this DC; Total 1000 times; 0.5 means outLocInfoRatio;
        unsigned Req = 1000, cost = 0, iReq = 1;
        for (iReq = 1; iReq <= 0.5*Req; ++iReq) {
            auto tmp = IU[rand()%usernum];//H
            DC Home = tmp.Loc;
            // not in home prob.
            if(rand()%100 > tmp.HomeInfoRatio) {
                auto Visitor = mobileuserlist.find(tmp.id);
                if(Visitor == mobileuserlist.end()) {
                    cout << "not found in mobile list" << endl;
                } else {
                    DC VisitPlace = Visitor.second;
                    //calculate the dijsktra[finded V][vector[DCcnt]]
                    //cost +=
                }
                //cost +=dijsktra[tmp.H][vector[DCnt]];
            }
        }
    }
    //cost for unique DC;
}

template<class DC>
void gen_dijkstraDC() {

}

template<class ID_Digest_len, class DC>
void testHLR() {
    //gen_dijkstraDC();

    LFSRGen<uint32_t> keyGen(0x1234567801234567ULL, usernum, 0);
    vector<user<ID_Digest_len, DC>> IU;

    for (unsigned i = 1; i <= usernum; i++) {
        user<ID_Digest_len, DC> tmp;
        keyGen.gen(&tmp.id);
        tmp.HomeLoc = rand() % dcnum;
        tmp.HomeInfoRatio = 0.5;
        if (i <= mobileuserrate * usernum) {
            tmp.VisitorLoc = rand() % dcnum;
            mobileuserlist.insert(pair<ID_Digest_len, DC>(tmp.id, tmp.VisitorLoc));
        } else
            tmp.VisitorLoc = tmp.HomeLoc;
        IU.push_back(tmp);
    }

    testStateofArt(IU);
    //testLudoHLF(keys, values, nn, zipfianKeys);

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


