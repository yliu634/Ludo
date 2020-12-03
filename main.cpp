#include "common.h"
#include "cstdlib"
#include "Othello/data_plane_othello.h"
#include "MinimalPerfectCuckoo/minimal_perfect_cuckoo.h"
#include "dijsktraform.h"
#include "home_visit_location.h"

using namespace std;

template<class ID, class DC>
struct user {
    bool isMobileUser;
    ID id;
    DC HomeLoc;
    DC VisitorLoc;
    double HomeInfoRatio = 0.5;
};

uint8_t usertype = 0;
uint32_t userdcnum = 360;
uint8_t dcnum = 14; //no more than 256;
uint32_t Totalusernum = userdcnum * dcnum;//42,94 million
//mistake for mobileuserrate, it is a ratio for (DV_H & DH_V users)/(DV_H + DH_V + D_H_V) ratio,
//it wouldnt be a problem if the number of user is big enough.
double mobileuserrate = 0.66;// 2-2-6 for 14 if 0.4. 3-3-4 for 0.6;
user<uint32_t, uint8_t> tmp1;
vector<vector<user<uint32_t, uint8_t>>> IU(dcnum, vector<user<uint32_t, uint8_t>> (userdcnum, tmp1)); //userlist including id and dc #

vector<vector<unsigned>> dijkstraDC(dcnum, vector<unsigned>(dcnum, 0));//dc distance form.
map<uint32_t, uint16_t> mobileuserlist;


template<class ID, class DC>
void genInquiryList(const DC idc, uint8_t usertype, const uint32_t req, vector<user<ID, DC>> &inquiryList){
    //usertype =0,1,2, maps to DH_V, DV_H, and D_H_V respectively.
    uint32_t bound1 = 0.5*mobileuserrate*userdcnum, bound2 = mobileuserrate*userdcnum;
    if (usertype == 0){
        for (uint32_t icnt = 1; icnt < req;icnt++)
            inquiryList.push_back(IU[idc][rand()%bound1]);
    } else if (usertype == 1){
        for (uint32_t icnt = 1; icnt < req;icnt++)
            inquiryList.push_back(IU[idc][rand()%(bound2-bound1) + bound1]);
    } else{
        for (uint32_t icnt = 1; icnt < req;icnt++)
            inquiryList.push_back(IU[idc][rand()%(userdcnum-bound2) + bound2]);
    }
}

template<class ID, class DC>
void testLudoHLR() {
//vector<ID> &keys, vector<DC> &values, uint64_t nn, vector<DC> &zipfianKeys){
//mpc.h 1005 line tell you what you found.
//in order to call cp(nn), mobile-list -> &keys, &values, nn;
    unsigned nn = Totalusernum, cost(0), Req(1000);
    //construct cp, 3th parameter is VL = uint16_t, 200~300 DCs, I hope it could be 8-length.
    //cout << mobileuserlist.size() << endl;
    ControlPlaneMinimalPerfectCuckoo<ID, DC> cp(nn);
    for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.cend(); iter++) {
        cp.insert(iter->first, iter->second);
    }
    cp.prepareToExport();
    //same as above with cp(nn)
    //DataPlaneMinimalPerfectCuckoo<ID, DC> dp(cp);

    for (unsigned iDCcnt = 0; iDCcnt < dcnum; ++iDCcnt) {
        //unsigned Req = 100, iReq = 1;
        vector<user<ID, DC>> inquiryList;
        genInquiryList<ID, DC>(iDCcnt, usertype, Req, inquiryList);
        for(const auto tmp:inquiryList) {
            ID id = tmp.id;
            DC Home = tmp.HomeLoc;
            DC LudoFindVisitor{1};
            //replace for a little while.
            //auto Visitor = mobileuserlist.find(tmp.id);
            //cp.lookUp(ID, LudoFindVisitor);
            //if ((Visitor == mobileuserlist.end())){//&&(!dp.lookUp(ID, LudoFindVisitor))) {
            if (!(cp.lookUp(id, LudoFindVisitor))) {
                //cout << "Not found in Mobile user list" << endl;
                cost += dijkstraDC[iDCcnt][Home];
            } else {
                //cost += (dijks[iDCcnt][tmp.H]>dijks[iDCcnt][LudoFindVisitor])?
                // 2 * dijkstra[iDCcnt][LudoFindVisitor]: 2 * dijkstra[iDCcnt][LudoFindVisitor];
                uint8_t Hflag = ((rand() % 100) <= 100 * tmp.HomeInfoRatio) ? 1 : 0;//info in home.
                uint8_t Hnear = dijkstraDC[iDCcnt][Home] < dijkstraDC[iDCcnt][LudoFindVisitor] ? 1 : 0;
                if (Hflag == 1 && Hnear == 1)
                    cost += dijkstraDC[iDCcnt][Home];
                else if (Hflag == 0 && Hnear == 0)
                    cost += dijkstraDC[iDCcnt][LudoFindVisitor];
                else
                    cost += dijkstraDC[iDCcnt][LudoFindVisitor] + dijkstraDC[iDCcnt][Home];
            }
        }
    }
    cost /= (Req * dcnum);
    cout << "Ludo: " << cost << endl;
}

template<class ID, class DC>
void testStateofArt() {
    unsigned Req(100), cost(0);
    for (unsigned iDCcnt = 0; iDCcnt < dcnum; ++iDCcnt) {
        //gen_request for every DC, OutLocInfoRatio percent:-> not this DC;
        //1-ratio percent:-> this DC; Total 1000 times; 0.5 means outLocInfoRatio;
        //unsigned Req = 10, cost = 0, iReq = 1;
        //for (unsigned iReq = 1; iReq <= Req; ++iReq) {
        vector<user<ID, DC>> inquiryList;
        genInquiryList<ID, DC>(iDCcnt, usertype, Req, inquiryList);
        for(const auto tmp:inquiryList) {
            //auto tmp = IU[rand() % Totalusernum];//H
            DC Home = tmp.HomeLoc;
            // not in home prob.
            //if (rand() % 100 > 100 * tmp.HomeInfoRatio) {
            auto Visitor = mobileuserlist.find(tmp.id);
            if (Visitor == mobileuserlist.end()) {
                //cout << "Not found in Mobile List" << endl;
            } else {
                DC VisitPlace = Visitor->second;
                //mobile user but info not in home thus we add cost, or we neednt.
                if (rand() % 100 > 100 * tmp.HomeInfoRatio)
                    cost += dijkstraDC[iDCcnt][VisitPlace];
            }
            //}every user need to go home register first.
            cost += dijkstraDC[iDCcnt][Home];
        }
    }
    cost /= (Req * dcnum);
    cout << "State of Art: " << cost << endl;
    cout << endl;
}

template<class ID, class DC>
void testHLR() {
    gen_dijkstraDC(dijkstraDC, dcnum);

    LFSRGen<uint32_t> keyGen(0x1234567801234567ULL, Totalusernum, 0);

    for (DC idc = 0; idc < dcnum; idc++) {
        ID iuserdc = 0;
        for (; iuserdc < 0.5 * mobileuserrate * userdcnum; iuserdc++) {
            user<ID, DC> tmp;
            keyGen.gen(&tmp.id);
            tmp.isMobileUser = 1;
            tmp.HomeLoc = idc;
            tmp.VisitorLoc = rand() % dcnum;
            while (tmp.VisitorLoc == idc)
                tmp.VisitorLoc = rand() % dcnum;
            IU[idc][iuserdc] = tmp;
            mobileuserlist.insert(pair<ID, DC>(tmp.id, tmp.VisitorLoc));
        }
        for (; iuserdc < mobileuserrate * userdcnum; iuserdc++) {
            user<ID, DC> tmp;
            keyGen.gen(&tmp.id);
            tmp.isMobileUser = 1;
            tmp.VisitorLoc = idc;
            tmp.HomeLoc = rand() % dcnum;
            while (tmp.HomeLoc == idc)
                tmp.HomeLoc = rand() % dcnum;
            IU[idc][iuserdc] = tmp;
            mobileuserlist.insert(pair<ID, DC>(tmp.id, tmp.VisitorLoc));
        }
        for (; iuserdc < userdcnum; iuserdc++) {
            user<ID, DC> tmp;
            keyGen.gen(&tmp.id);
            tmp.isMobileUser = 0;
            tmp.HomeLoc = rand() % dcnum;
            while (tmp.HomeLoc == idc)
                tmp.HomeLoc = rand() % dcnum;
            tmp.VisitorLoc = rand() % dcnum;
            while (tmp.VisitorLoc == idc)
                tmp.VisitorLoc = rand() % dcnum;
            IU[idc][iuserdc] = tmp;
            mobileuserlist.insert(pair<ID, DC>(tmp.id, tmp.VisitorLoc));
        }
    }
/*    //vector<user<ID, DC>> inquiryList;
    genInquiryList<ID, DC>(1, 1, 10, inquiryList);
    cout << " "<<endl;*/
    /*
    for (unsigned i = 1; i <= Totalusernum; i++) {
        user<ID, DC> tmp;
        keyGen.gen(&tmp.id);
        tmp.HomeLoc = rand() % dcnum;
        if (i <= mobileuserrate * Totalusernum) {
            tmp.VisitorLoc = rand() % dcnum;//mobile user list gen.
            mobileuserlist.insert(pair<ID, DC>(tmp.id, tmp.VisitorLoc));
            tmp.HomeInfoRatio = 0.5;
        } else {
            tmp.HomeInfoRatio = 1.0;
            tmp.VisitorLoc = tmp.HomeLoc;
        }
        IU.push_back(tmp);
    }
    */
    testStateofArt<ID, DC>();
    testLudoHLR<ID, DC>();

}

int main(int argc, char **argv) {
    commonInit();
    testHLR<uint32_t, uint8_t>();
}


