//.
#pragma once

#include "common.h"

template<class ID, class DC>
struct user {
    bool isMobileUser = true;
    ID id{0};
    DC HomeLoc{0};
    DC VisitorLoc{0};
    float HomeInfoRatio = 0.3;
};

template<class ID, class DC>
class LudoNearStateofArt {

public:
    ostringstream oss;
    DC usertype;
    ID Totalusernum;
    typedef uint16_t FP;
    const DC dcnum = 99; //variable any number;
    const ID userdcnum = Totalusernum / dcnum;
    map<ID, DC> mobileuserlist;
    //double mobileuserrate = 0.66; //
    vector<vector<user<ID, DC>>> IU;//(dcnum, vector<user<ID, uint8_t>>(userdcnum,
    vector<vector<uint16_t>> dijkstraDC;
    ControlPlaneMinimalPerfectCuckoo<ID, DC> cp;
    //DataPlaneMinimalPerfectCuckoo<ID, DC> dp;

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
      printf("ey\n");
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
      //usertype = 1, 2, maps to DV_H, and D_H_V respectively.
      /*
      uint32_t bound = 0.5 * userdcnum;
      if (usertype == 1) {
        for (uint32_t icnt = 1; icnt < req; icnt++)
          inquiryList.push_back(IU[idc][rand() % bound]);
      } else {
        for (uint32_t icnt = 1; icnt < req; icnt++)
          inquiryList.push_back(IU[idc][rand() % bound + bound]);
      }
       */
      for (uint32_t icnt = 1; icnt < req; icnt++)
        inquiryList.push_back(IU[idc][rand() % userdcnum]);

    }

    void genInquiryListZipfian(const DC idc, const uint32_t req, vector<user<ID, DC>> &inquiryList) {
      //usertype = 1, 2, maps to DV_H, and D_H_V respectively.
      /*
      uint32_t bound = 0.5 * userdcnum;
      if (usertype == 1) {
        for (uint32_t icnt = 1; icnt < req; icnt++)
          inquiryList.push_back(IU[idc][rand() % bound]);
      } else {
        for (uint32_t icnt = 1; icnt < req; icnt++)
          inquiryList.push_back(IU[idc][rand() % bound + bound]);
      }
      */
      uint32_t icnt = 0;
      for (icnt = 0; icnt < 0.2 * req; icnt++)
        for (uint8_t loop28 = 0; loop28 < 4; loop28 ++)
          inquiryList.push_back(IU[idc][rand() % userdcnum]);
      for (icnt = 0; icnt < 0.2 * req; icnt++)
        inquiryList.push_back(IU[idc][rand() % userdcnum]);

    }

    void testLudoHLR() {
      uint32_t nn(Totalusernum), cost(0), Req(1000);
      //ControlPlaneMinimalPerfectCuckoo<ID, DC> cp(nn);
      cp.Clear(nn);
      for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.cend(); iter++) {
        cp.insert(iter->first, iter->second);
      }
      //cp.insert(444,4);
      cp.prepareToExport();
      DataPlaneMinimalPerfectCuckoo<ID, DC> dp(cp);
      //DC aaa;
      //int flagg = dp.lookUp(444,aaa);
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

          if (!(dp.lookUp(id, LudoFindVisitor))) {
            //cout << "Not found in Mobile user list" << endl;
            cost += dijkstraDC[iDCcnt][Home];
          } else {
            //cost += (dijks[iDCcnt][tmp.H]>dijks[iDCcnt][LudoFindVisitor])?
            // 2 * dijkstra[iDCcnt][LudoFindVisitor]: 2 * dijkstra[iDCcnt][LudoFindVisitor];
            /*哪个近去哪儿
            uint8_t Hflag = ((rand() % 100) <= 100 * tmp.HomeInfoRatio) ? 1 : 0;//info in home.
            //uint8_t Hflag = 0;
            uint8_t Hnear = dijkstraDC[iDCcnt][Home] < dijkstraDC[iDCcnt][LudoFindVisitor] ? 1 : 0;
            if (Hflag == 1 && Hnear == 1)
              cost += dijkstraDC[iDCcnt][Home];
            else if (Hflag == 0 && Hnear == 0)
              cost += dijkstraDC[iDCcnt][LudoFindVisitor];
            else
              cost += dijkstraDC[iDCcnt][LudoFindVisitor] + dijkstraDC[iDCcnt][Home];
            */
            cost += dijkstraDC[iDCcnt][LudoFindVisitor];
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
    }

    void testStateArt() {

      uint32_t Req(1000), cost(0);
      //Clocker soatime("State of Art");
      for (DC iDCcnt = 0; iDCcnt < dcnum; ++iDCcnt) {
        //gen_request for every DC, OutLocInfoRatio percent:-> not this DC;
        //for (unsigned iReq = 1; iReq <= Req; ++iReq) {
        vector<user<ID, DC>> inquiryList;
        genInquiryList(iDCcnt, Req, inquiryList);
        //genInquiryListZipfian(iDCcnt, Req, inquiryList);
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
            /*if (rand() % 100 > 100 * tmp.HomeInfoRatio)
            没那么多事儿，俩都得加上;
             */
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

    }

    void testStateArt2() {

      uint32_t Req(1000), cost(0);
      //Clocker soatime("State of Art");
      for (DC iDCcnt = 0; iDCcnt < dcnum; ++iDCcnt) {
        //gen_request for every DC, OutLocInfoRatio percent:-> not this DC;
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
            /*if (rand() % 100 > 100 * tmp.HomeInfoRatio)
            没那么多事儿，俩都得加上;
             */
            //三角路由
            cost += dijkstraDC[iDCcnt][VisitPlace]/2 + dijkstraDC[Home][VisitPlace]/2;
          }
          //}every user need to go home register first.
          cost += dijkstraDC[iDCcnt][Home]/2;
        }
      }
      //soatime.stop();
      cost /= (Req * dcnum);
      //uint64_t mem = mobileuserlist().size()*sizeof(mobileuserlist.begin());
      oss << "TotalUserNum: " << Totalusernum << endl;
      oss << "State of Art RTT2: " << cost << endl;
      oss << "Memory Cost: " << getMemoryStateofArt() << endl;
      cout << oss.str() << endl;
      oss.str("");
      cout << oss.str() << endl;

    }

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
      DataPlaneMinimalPerfectCuckoo<ID, DC> dp(cp);

      vector<pair<vector<MPC_PathEntry>, pair<DC, FP>>> insertPaths;
      vector<pair<uint32_t, pair<DC, FP>>> modifications;
      LFSRGen<ID> keyGen(0x1234567801234667ULL, 1U << 30, 0);
      // prepare many updates. modification : insertion : deletion = 1:1:1

      for (uint16_t i = 0; i < mobileuserlist.size(); i++) {
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
          DC v = 99;
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
      }

      //test for update/modification.
      //DataPlaneMinimalPerfectCuckoo<ID, DC> dp(cp);
      /*
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
        cout << "flaggg" << endl;
      }
      */
      //test for insert;
      /*
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

      // Clocker c("MPC apply " + to_string(lookupCnt) + " updates");
      for (uint16_t i = 0; i < mobileuserlist.size(); i++) {
        if (i % 3 == 0) {  // delete
          // empty
        } else if (i % 3 == 1) { // modify
          pair<uint32_t, pair<DC, FP>> tmp = modifications.at(i / 3);
          dp.applyUpdate(tmp.first, tmp.second.first, tmp.second.second);
        } else {// insert
          pair<vector<MPC_PathEntry>, pair<DC, FP>> tmp = insertPaths.at(i / 3);
          dp.applyInsert(tmp.first, tmp.second.first, tmp.second.second);
          //int falgg = dp.lookUp();
        }
      }
      cout << "thanks folks!" << endl;


    }

    vector<DC> NearNodeList(DC failServer, DC num) {
      vector<DC> array(dcnum, 0);
      vector<DC> res;
      for (DC i = 0; i < dcnum; i++)
        array[i] = dijkstraDC[failServer][i];
      for (DC i = 0; i <= num; i++) {
        auto min = min_element(array.begin(), array.end());
        *min = 65535;
        if (i == 0)
          continue;
        else
          res.push_back((min - array.begin()));
      }
      return res;
    }

    void testAgentNode() {
      //approach is that:
      /*
       * choose one user, choose a place P that is different from V and H, then, if V, rtt += (V-> P) and
       * h dont know or know, to go near one; Concurrent N users for same situation;
       *  --------------------------------------------
       *  |         |  before fail   |   after fail  |
       *  |  D_V_H  |     cost[0]    |    cost[1]    |
       *  |State_Art|     cost[2]    |    cost[3]    |
       *  --------------------------------------------
       */
      ControlPlaneMinimalPerfectCuckoo<ID, DC> cp(Totalusernum);
      for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.cend(); iter++) {
        cp.insert(iter->first, iter->second);
      }
      cp.prepareToExport();
      //set rtt range for DV_H, for example, DV is near, rtt <= 50 ms;
      DataPlaneMinimalPerfectCuckoo<ID, DC> dp(cp);

      //choose the failed server number;
      DC failServerNum = 16;
      DC NearServerNum = 4;
      vector<DC> failServerList;

      for (DC iFailServer = 0; iFailServer < failServerNum; iFailServer++) {
        DC tmp = rand() % dcnum;
        auto it = find(failServerList.begin(), failServerList.end(), tmp);
        while (true) {  //not in fail Server List
          if (it == failServerList.end())
            break;
          tmp = rand() % dcnum;
          it = find(failServerList.begin(), failServerList.end(), tmp);
        }
        failServerList.push_back(tmp);
      }

      //for (const auto failServer: failServerList) {

      //find the x th closet server to failServer; a vector contains x numbers;
      DC iNearServer = 0;
      vector<DC> NearServer(NearServerNum, 0);
      vector<pair<uint32_t, pair<DC, FP>>> modifications;

      for (const auto failServer: failServerList) {
        NearServer = NearNodeList(failServer, NearServerNum);
        for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.cend(); iter++) {
          //only change users that V is failServer;
          if (iter->second == failServer) {
            //change in control plane;
            FP finger = FastHasher64<ID>(0)(iter->first) >> 48;

            DC v = NearServer[iNearServer % NearServerNum];
            //if v in Fail-Server list;
            auto it = find(failServerList.begin(), failServerList.end(), v);
            int count = 0;
            while (it != failServerList.end()) {
              count++;
              iNearServer++;
              v = NearServer[iNearServer % NearServerNum];
              it = find(failServerList.begin(), failServerList.end(), v);
              assert(count <= 3);
            }

            cp.updateMapping(iter->first, v);
            pair<uint32_t, uint32_t> tmp = cp.locate(iter->first);
            uint8_t sid = FastHasher64<ID>(cp.buckets_[tmp.first].seed)(iter->first) >> 62;
            modifications.emplace_back((tmp.first << 2) + sid, pair<ID, FP>(v, finger));
            //not change in mobileuserlist, just in control plane and data plane;
            iNearServer++;
          }
        }
      }

      // DataPlaneMinimalPerfectCuckoo<ID, DC> dp(cp);
      // update in data control plane;
      for (ID i = 0; i < modifications.size(); i++) {
        auto tmp = modifications.at(i);
        dp.applyUpdate(tmp.first, tmp.second.first, tmp.second.second);
      }

      //lookUp and implement rtt evaluation
      //gen inquiryList that V = failServer, H nobody care;
      uint32_t Req(100), inquiryListSize(0);
      vector<user<ID, DC>> inquiryList;     //D_H_V
      vector<uint32_t> cost(4, 0);


      //for (const auto failServer:failServerList) {

      //inline func inquiryList;
      for (DC idc = 0; idc < dcnum; idc++) {
        for (ID j = 0; j < userdcnum; j++) {
          auto tmp = IU[idc][j];
          auto itDC = find(failServerList.begin(),failServerList.end(),tmp.VisitorLoc);
          if (itDC != failServerList.end())
            inquiryList.push_back(tmp);
        }
      }
      inquiryListSize += inquiryList.size();


      for (DC iDCcnt = 0; iDCcnt < dcnum; ++iDCcnt) {

        //unsigned Req = 100, iReq = 1
        /*
        if (iDCcnt == failServer) {

          //condition that D = V;
          for (const auto tmp:inquiryList) {
            ID id = tmp.id;
            DC Home = tmp.HomeLoc;
            DC LudoFindVisitor{0};

            if (!(dp.lookUp(id, LudoFindVisitor))) {
              //cout << "Not found in Mobile user list" << endl;
              assert(0);
              cost[0] += dijkstraDC[iDCcnt][Home];
              cost[1] += dijkstraDC[iDCcnt][Home];
            } else {
              //cost += (dijks[iDCcnt][tmp.H]>dijks[iDCcnt][LudoFindVisitor])?
              // 2 * dijkstra[iDCcnt][LudoFindVisitor]: 2 * dijkstra[iDCcnt][LudoFindVisitor];
              uint8_t Hflag = ((rand() % 100) <= 100 * tmp.HomeInfoRatio) ? 1 : 0;//info in home.
              uint8_t Hnear = dijkstraDC[iDCcnt][Home] < dijkstraDC[iDCcnt][LudoFindVisitor] ? 1 : 0;
              if (Hflag == 1 && Hnear == 1) {
                cost[0] += dijkstraDC[iDCcnt][Home];
                cost[1] += dijkstraDC[iDCcnt][Home];
              } else if (Hflag == 0 && Hnear == 0) {
                cost[0] += dijkstraDC[iDCcnt][failServer];
                cost[1] += (dijkstraDC[iDCcnt][failServer] + dijkstraDC[LudoFindVisitor][failServer]);
              } else {
                cost[0] += dijkstraDC[iDCcnt][failServer] + dijkstraDC[iDCcnt][Home];
                cost[1] += dijkstraDC[iDCcnt][failServer] + dijkstraDC[iDCcnt][Home] +
                           dijkstraDC[LudoFindVisitor][failServer];
              }
            }
          }
        } else {
          */

        auto itDC = find(failServerList.begin(),failServerList.end(),iDCcnt);
        if (itDC != failServerList.end())
          continue;

        for (const auto tmp:inquiryList) {
          ID id = tmp.id;
          DC Home = tmp.HomeLoc;
          DC failServer = tmp.VisitorLoc;
          DC LudoFindVisitor{0};

          if (!(dp.lookUp(id, LudoFindVisitor))) {
            //cout << "Not found in Mobile user list" << endl;
            assert(0);
            cost[2] += dijkstraDC[iDCcnt][Home];
            cost[3] += dijkstraDC[iDCcnt][Home];
          } else {
            //cost += (dijks[iDCcnt][tmp.H]>dijks[iDCcnt][LudoFindVisitor])?
            // 2 * dijkstra[iDCcnt][LudoFindVisitor]: 2 * dijkstra[iDCcnt][LudoFindVisitor];
            uint8_t Hflag = ((rand() % 100) <= 100 * tmp.HomeInfoRatio) ? 1 : 0;//info in home.
            uint8_t Hnear = dijkstraDC[iDCcnt][Home] < dijkstraDC[iDCcnt][LudoFindVisitor] ? 1 : 0;
            if (Hflag == 1 && Hnear == 1) {
              cost[0] += dijkstraDC[iDCcnt][Home];
              cost[1] += dijkstraDC[iDCcnt][Home];
              cost[2] += dijkstraDC[iDCcnt][Home];
              cost[3] += dijkstraDC[iDCcnt][Home];
            } else if (Hflag == 0 && Hnear == 0) {
              cost[0] += dijkstraDC[iDCcnt][failServer];
              cost[1] += (dijkstraDC[iDCcnt][failServer] + dijkstraDC[LudoFindVisitor][failServer]);
              cost[2] += dijkstraDC[iDCcnt][failServer] + dijkstraDC[iDCcnt][Home];
              cost[3] += (dijkstraDC[iDCcnt][failServer] + dijkstraDC[LudoFindVisitor][failServer] +
                          dijkstraDC[iDCcnt][Home]);
            } else if (Hflag == 1 && Hnear == 0) {
              cost[0] += dijkstraDC[iDCcnt][failServer] + dijkstraDC[iDCcnt][Home];
              cost[1] += dijkstraDC[iDCcnt][failServer] + dijkstraDC[iDCcnt][Home] +
                         dijkstraDC[LudoFindVisitor][failServer];
              cost[2] += dijkstraDC[iDCcnt][Home];
              cost[3] += dijkstraDC[iDCcnt][Home];
            } else {
              cost[0] += dijkstraDC[iDCcnt][failServer] + dijkstraDC[iDCcnt][Home];
              cost[1] += dijkstraDC[iDCcnt][failServer] + dijkstraDC[iDCcnt][Home] +
                         dijkstraDC[LudoFindVisitor][failServer];
              cost[2] += dijkstraDC[iDCcnt][Home] + dijkstraDC[iDCcnt][failServer] +
                         dijkstraDC[LudoFindVisitor][failServer];
              cost[3] += dijkstraDC[iDCcnt][Home] + dijkstraDC[iDCcnt][failServer] +
                         dijkstraDC[LudoFindVisitor][failServer];

            }
          }//else

        }// for

      }//DC loop


      cout << " - Uaena - BBIBBI - RTT: " << Totalusernum << endl;
      //cost[0] /= dcnum * inquiryListSize;
      //cost[1] /= dcnum * inquiryListSize;
      for (auto el: cost) {
        //el /= inquiryList.size();
        el /= (dcnum - failServerNum) * inquiryListSize;
        cout << "D_V_H:   " << el << endl;
      }
      cout << endl;

    }

    void test() {
      testStateArt();
      testStateArt2();
      testLudoHLR();
      //testLudoUpdate();
      //testAgentNode();
    }

};

