//.
#pragma once

#include "common.h"

//
//
//
template<class ID, class DC>
struct user {
    bool isMobileUser = true;
    ID id{0};
    DC HomeLoc{0};
    DC VisitorLoc{0};
    float HomeInfoRatio = 0.5;
};

template<class ID, class DC, DC usertype = 1, ID Totalusernum = 16000>
class LudoNearStateofArt {

public:
    ostringstream oss;
    const DC dcnum = 14; //no more than 256;
    const ID userdcnum = Totalusernum / 14;
    //double mobileuserrate = 0.66; // 2-2-6 for 14 if 0.4. 3-3-4 for 0.6;

    vector<vector<user<ID, uint8_t>>> IU;//(dcnum, vector<user<ID, uint8_t>>(userdcnum,
    vector<vector<uint32_t>> dijkstraDC;
    map<ID, uint8_t> mobileuserlist;

    explicit LudoNearStateofArt() {
        Clear();
    }

    void Clear() {

        vector<uint32_t> empty_dijs(userdcnum, 0);
        dijkstraDC.clear();
        dijkstraDC.resize(dcnum, empty_dijs);
        gen_dijkstraDC(dijkstraDC, dcnum);

        user<ID, DC> empty_user;
        //empty_user.clear();
        IU.clear();
        IU.resize(dcnum, vector<user<ID, uint8_t>>(userdcnum, empty_user));

        MobileUserListInit();
    }

    void MobileUserListInit() {
        LFSRGen<uint32_t> keyGen(0x1234567801234567ULL, 1U << 30, 0); //Totalusernum, 0);

        for (DC idc = 0; idc < dcnum; idc++) {
            ID iuserdc = 0;
            for (; iuserdc < 0.33 * userdcnum; iuserdc++) {
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
            for (; iuserdc < 0.66 * userdcnum; iuserdc++) {
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
                while (tmp.VisitorLoc == idc || tmp.VisitorLoc == tmp.HomeLoc)
                    tmp.VisitorLoc = rand() % dcnum;
                IU[idc][iuserdc] = tmp;
                mobileuserlist.insert(pair<ID, DC>(tmp.id, tmp.VisitorLoc));
            }
        }//end all DCs
    }

    uint32_t getMemoryStateofArt() {
        uint32_t mem = mobileuserlist.size() * sizeof(mobileuserlist.begin());
        //std::cout << "State of Art memory: " << mem << std::endl;
        return mem;
    }

    void genInquiryList(const DC idc, const uint32_t req, vector<user<ID, DC>> &inquiryList) {
        //usertype =0,1,2, maps to DH_V, DV_H, and D_H_V respectively.
        //
        uint32_t bound1 = 0.33 * userdcnum, bound2 = 0.66 * userdcnum;
        if (usertype == 0) {
            for (uint32_t icnt = 1; icnt < req; icnt++)
                inquiryList.push_back(IU[idc][rand() % bound1]);
        } else if (usertype == 1) {
            for (uint32_t icnt = 1; icnt < req; icnt++)
                inquiryList.push_back(IU[idc][rand() % (bound2 - bound1) + bound1]);
        } else {
            for (uint32_t icnt = 1; icnt < req; icnt++)
                inquiryList.push_back(IU[idc][rand() % (userdcnum - bound2) + bound2]);
        }
    }//end

    void testLudoHLR() {

        uint32_t nn = Totalusernum, cost(0), Req(1000);
        ControlPlaneMinimalPerfectCuckoo<ID, DC> cp(nn);
        for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.cend(); iter++) {
            cp.insert(iter->first, iter->second);
        }
        cp.prepareToExport();
        for (DC iDCcnt = 0; iDCcnt < dcnum; ++iDCcnt) {
            //unsigned Req = 100, iReq = 1;
            vector<user<ID, DC>> inquiryList;
            genInquiryList(iDCcnt, Req, inquiryList);
            for (const auto tmp:inquiryList) {
                ID id = tmp.id;
                DC Home = tmp.HomeLoc;
                DC LudoFindVisitor{0};
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
        //cpBuild.stop();
        cost /= (Req * dcnum);
        oss << "Ludo RTT: " << cost << endl;
        oss << "Memory cost: " << cp.getMemoryCost() << endl;
        cout << oss.str() << endl;
        oss.clear();
        // x uint8_t one byte.

    }//end ludo;

    void testStateofArt() {

        uint32_t Req(100), cost(0);
        //Clocker soatime("State of Art");
        for (DC iDCcnt = 0; iDCcnt < dcnum; ++iDCcnt) {
            //gen_request for every DC, OutLocInfoRatio percent:-> not this DC;
            //1-ratio percent:-> this DC; Total 1000 times; 0.5 means outLocInfoRatio;
            //unsigned Req = 10, cost = 0, iReq = 1;
            //for (unsigned iReq = 1; iReq <= Req; ++iReq) {
            vector<user<ID, DC>> inquiryList;
            genInquiryList(iDCcnt, Req, inquiryList);
            for (const auto tmp:inquiryList) {
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
        //soatime.stop();
        cost /= (Req * dcnum);
        //uint64_t mem = mobileuserlist().size()*sizeof(mobileuserlist.begin());
        oss << "TotalUserNum: " << Totalusernum << endl;
        oss << "State of Art RTT: " << cost << endl;
        oss << "Memory Cost: " << getMemoryStateofArt() << endl;
        cout << oss.str() << endl;
        oss.str("");
        cout <<oss.str()<<endl;

    }//end state of art

    void test() {
        testStateofArt();
        testLudoHLR();
    }

};
