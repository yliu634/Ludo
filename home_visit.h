//.
#pragma once

#include "common.h"

template<class ID, class DC>
struct user {
    bool isMobileUser = true;
    ID id{0};
    DC HomeLoc{0};
    DC VisitorLoc{0};
    float HomeInfoRatio = 0.5;
};

template<class ID, class DC>
class LudoNearStateofArt {

public:
    ostringstream oss;
    DC usertype;
    ID Totalusernum;
    typedef uint16_t FP;
    const DC dcnum = 20; //no more than 256;
    const ID userdcnum = Totalusernum / dcnum;
    //double mobileuserrate = 0.66; // 2-2-6 for 14 if 0.4. 3-3-4 for 0.6;
    vector<vector<user<ID, DC>>> IU;//(dcnum, vector<user<ID, uint8_t>>(userdcnum,
    vector<vector<uint16_t>> dijkstraDC;
    map<ID, DC> mobileuserlist;
    ControlPlaneMinimalPerfectCuckoo<ID, DC> *cptr;
    DataPlaneMinimalPerfectCuckoo<ID, DC> *dptr;

    explicit LudoNearStateofArt(uint8_t ty, ID Totl) : usertype(ty), Totalusernum(Totl) {
      Clear();
    }

    void Clear() {

      vector<DC> empty_dijs(dcnum, 0);
      dijkstraDC.clear();
      dijkstraDC.resize(dcnum, empty_dijs);
      gen_dijkstraDC(dijkstraDC, dcnum);

      user<ID, DC> empty_user;
      //empty_user.clear();
      IU.clear();
      IU.resize(dcnum, vector<user<ID, DC>>(userdcnum, empty_user));

      MobileUserListInit();
    }

    void MobileUserListInit() {
      LFSRGen<ID> keyGen(0x1234567801234567ULL, 1U << 30, 0); //Totalusernum, 0);

      for (DC idc = 0; idc < dcnum; idc++) {
        ID iuserdc = 0;
        for (; iuserdc < 0.5 * userdcnum; iuserdc++) {
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

      uint32_t bound = 0.5 * userdcnum;
      if (usertype == 1) {
        for (uint32_t icnt = 1; icnt < req; icnt++)
          inquiryList.push_back(IU[idc][rand() % bound]);
      } else {
        for (uint32_t icnt = 1; icnt < req; icnt++)
          inquiryList.push_back(IU[idc][rand() % bound + bound]);
      }
    }//end

    void AddMobileUser(pair<ID, DC> &p) {
      //pair<ID, DC> aa(34144134, 35);
      (*cptr).insert(p.first, p.second);
    }

    void RemoveMobileUser(ID &target) {
      (*cptr).remove(target);
    }

    void testLudoHLR() {
      uint32_t nn(Totalusernum), cost(0), Req(1000);
      ControlPlaneMinimalPerfectCuckoo<ID, DC> cp(nn);
      for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.cend(); iter++) {
        cp.insert(iter->first, iter->second);
      }
      //cp.insert(444,4);
      cp.prepareToExport();
      DataPlaneMinimalPerfectCuckoo<ID, DC> dp(cp);
      cptr = &cp;
      dptr = &dp;
      //cp.insert(444,4);
      //DC aaa;int flagg = dp.lookUp(444,aaa);
      //int flaggg = cp.lookUp(444,aaa);
      for (DC iDCcnt = 0; iDCcnt < dcnum; ++iDCcnt) {
        //unsigned Req = 100, iReq = 1;
        vector<user<ID, DC>> inquiryList;
        genInquiryList(iDCcnt, Req, inquiryList);
        for (const auto tmp:inquiryList) {
          ID id = tmp.id;
          DC Home = tmp.HomeLoc;
          DC LudoFindVisitor{0};
          //replace for a little while.
          //cp.lookUp(ID, LudoFindVisitor);
          //if ((Visitor == mobileuserlist.end())){//&&(!dp.lookUp(ID, LudoFindVisitor))) {
          if (!(dp.lookUp(id, LudoFindVisitor))) {
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
      oss << "Memory cost: " << cp.getMemoryCost() << endl << "--------------";
      //AddMobileUser();
      cout << oss.str() << endl;

      oss.clear();
      // x uint8_t one byte.
    }//end ludo;

    void testStateArt() {

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
      cout << oss.str() << endl;

    }//end state of art

    ~LudoNearStateofArt() {
    }

    void testLudoUpdate() {

      ControlPlaneMinimalPerfectCuckoo<ID, DC> cp(Totalusernum);
      for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.cend(); iter++) {
        cp.insert(iter->first, iter->second);
      }
      /*
        for (int i = Totalusernum * 2 / 3; i < Totalusernum; ++i) {
        (*cptr).remove(mobileuserlist.begin()->first);
        }
      */
      cp.prepareToExport();
      //dptr = &dp;
      vector<pair<vector<MPC_PathEntry>, pair<DC, FP>>> insertPaths;
      vector<pair<uint32_t, pair<DC, FP>>> modifications;
      LFSRGen<ID> keyGen(0x1234567801234667ULL, 1U << 30, 0);
      // prepare many updates. modification : insertion : deletion = 1:1:1
      uint8_t i = 0;
      for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.end(); iter++) {
        if (i % 3 == 0) {
          ID k;
          while (true) {
            k = IU[rand() % dcnum][rand() % userdcnum].id;
            DC tmp;
            if (cp.lookUp(k, tmp)) {
              break;
            }
          }
          cp.remove(k);
        } else if (i % 3 == 1) {
          ID k;
          while (true) {
            k = IU[rand() % dcnum][rand() % userdcnum].id;
            DC tmp;
            if (cp.lookUp(k, tmp)) {
              break;
            }
          }
          FP finger = FastHasher64<ID>(0)(k) >> 48;
          DC v = rand() % dcnum;
          cp.updateMapping(k, v);
          pair<uint32_t, uint32_t> tmp = cp.locate(k);
          uint8_t sid = FastHasher64<ID>(cp.buckets_[tmp.first].seed)(k) >> 62;
          modifications.emplace_back((tmp.first << 2) + sid, pair<ID, FP>(v, finger));
        } else { //insert
          ID k;
          while (true) {
            keyGen.gen(&k);
            //k = IU[rand() % dcnum][rand() % userdcnum].id;
            DC tmp;
            if (!(cp.lookUp(k, tmp))) {
              break;
            }
          }
          DC v = rand() % dcnum;
          FP finger = FastHasher64<ID>(0)(k);
          vector<MPC_PathEntry> path;
          //cp.insert(k, v);
          cp.insert(k, v, &path);
          insertPaths.emplace_back(path, pair<ID, DC>(v, finger));
          //dp.applyInsert(path, v, finger);

          uint32_t s = 0;
          for (auto e: path) {
            s += e.locatorCC.size() * 4;
          }
          //counter()
        }
        i++;
      }

      {
        DataPlaneMinimalPerfectCuckoo<ID, DC> dp(cp);
        /* test for update/modification.
        {
          DC aaa;
          int flaggg;
          ID mk = IU[rand() % dcnum][rand() % userdcnum].id;
          pair<uint32_t, uint32_t> tmp = cp.locate(mk);
          flaggg = dp.lookUp(mk, aaa);
          DC v = 99;
          FP finger = FastHasher64<ID>(0)(mk) >> 48;
          cp.updateMapping(mk, v);
          tmp = cp.locate(mk);
          uint64_t seed = cp.buckets_[tmp.first].seed;
          uint8_t sid = FastHasher64<ID>(seed)(mk) >> 62;
          dp.applyUpdate((tmp.first << 2) + sid, v, finger);
          flaggg = dp.lookUp(mk, aaa);
          cout << "flaggg"<<endl;
        }
        */
        /*
        //test for insert;
        {
          DC aaa;
          int flaggg;
          ID mk = 89;
          flaggg = dp.lookUp(mk, aaa);
          vector<MPC_PathEntry> path;
          DC v = 18;
          FP finger = FastHasher64<ID>(0)(mk) >> 48;
          cp.insert(mk, v, &path);
          //insertPaths.emplace_back(path, pair<ID, DC>(v, finger));
          dp.applyInsert(path, v, finger);
          flaggg = dp.lookUp(mk, aaa);
          cout << "flaggg"<<endl;
        }
        */

        i = 0;
        //Clocker c("MPC apply " + to_string(lookupCnt) + " updates");
        for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.end(); iter++) {
          if (i % 3 == 0) { // delete
            // empty
          } else if (i % 3 == 1) { // modify
            pair<uint32_t, pair<DC, FP>> tmp = modifications.at(i / 3);
            dp.applyUpdate(tmp.first, tmp.second.first, tmp.second.second);
          } else {// insert
            pair<vector<MPC_PathEntry>, pair<DC, FP>> tmp = insertPaths.at(i / 3);
            dp.applyInsert(tmp.first, tmp.second.first, tmp.second.second);
            //int falgg = dp.lookUp();
          }
          i++;
        }
      }
    }//end benchmark

    void test() {
      //testStateArt();
      //testLudoHLR();
      testLudoUpdate();
    }

};

