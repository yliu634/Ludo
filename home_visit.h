//.
#pragma once

#include "common.h"

template<class ID, class DC>
struct user {
    bool isMobileUser = true;
    ID id{0};
    DC HomeLoc{0};
    uint64_t VisitorBitmap{0};
    float HomeInfoRatio = 0.3;
};

template<class ID, class DC>
class LudoNearStateofArt {
public:
    DC usertype;
    ID Totalusernum;

    typedef uint32_t BP; // the bitmap class
    typedef uint16_t FP;
    const DC dcnum = 99; //variable any number;
    const ID userdcnum = Totalusernum / dcnum;

    map<ID, user<ID, DC>> mobileuserlist;
    vector<vector<uint16_t>> dijkstraDC;
    vector<vector<user<ID, DC>>> IU;

    explicit LudoNearStateofArt(ID Totl) : Totalusernum(Totl) {
      Clear();
    }

    void Clear() {

      user<ID, DC> empty_user;
      vector<DC> empty_dijs(dcnum, 0);
      dijkstraDC.clear();
      dijkstraDC.resize(dcnum, empty_dijs);
      gen_dijkstraDC(dijkstraDC, dcnum);

      IU.clear();
      IU.resize(dcnum, vector<user<ID, DC>>(userdcnum, empty_user));

      MobileUserListInit();
    }

    void MobileUserListInit() {
      LFSRGen<ID> keyGen(0x1234567801234567ULL, 1U << 30, 0); //Totalusernum

      for (DC idc = 0; idc < dcnum; idc++) {
        for (ID iuserdc = 0; iuserdc < userdcnum; iuserdc++) {

          DC Vtmp;
          user<ID, DC> tmp;

          keyGen.gen(&tmp.id);
          tmp.isMobileUser = 1;
          tmp.HomeLoc = idc;
          tmp.VisitorBitmap = genRandNum(3, dcnum, idc);

          IU[idc][iuserdc] = tmp;
          mobileuserlist.insert(pair<ID, user<ID, DC>>(tmp.id, tmp));
        }
      }

      cout << "MobileUserList completed." << endl;
    }//end

    void testExtension() {

      ControlPlaneMinimalPerfectCuckoo<ID, BP> cp(Totalusernum);
      for (auto iter = mobileuserlist.cbegin(); iter != mobileuserlist.cend(); iter++) {
        cp.insert(iter->first, iter->second.VisitorBitmap);
      }
      cp.prepareToExport();
      DataPlaneMinimalPerfectCuckoo<ID, BP> dp(cp);

      vector<ID> InquiryList;
      vector<uint64_t> cost(2, 0);
      BP VisitBitmap(0);

      for (DC iDC = 0; iDC < dcnum; iDC++) {
        InquiryList.clear();
        GenInquiryListForEveryDC(InquiryList, iDC);
        //cout << endl;
        for (const auto el: InquiryList) {
          auto iter = mobileuserlist.find(el);
          auto Home = iter->second.HomeLoc;
          dp.lookUp(el, VisitBitmap);
          auto tmpCost = CalculateRttCost(iDC, Home, VisitBitmap);
          cost[0] += tmpCost[0];
          cost[1] += tmpCost[1];
        }
      }

      cost[0] /= (dcnum * InquiryList.size());
      cost[1] /= (dcnum * InquiryList.size());
      cout << endl;
      cout << "Total Number of DCs: " << dcnum << endl;
      cout << "The Ludo and Near Rtt cost is: " << cost[0] << endl;
      cout << "The State of the art Rtt cost is: " << cost[1] << endl;
      cout << endl;
    }

    inline uint64_t genRandNum(int num, DC dcnum, DC iDC) {

      uint64_t Vtmp;
      set<DC> V = {iDC};
      uint64_t VBitmap{0};
      for (int iV = 0; iV < num; iV++) {
        Vtmp = rand() % dcnum;
        V.insert(Vtmp);
        while (V.size() < iV + 2) {
          Vtmp = rand() % dcnum;
          V.insert(Vtmp);
        }
        VBitmap += (1U << Vtmp);
      }
      return VBitmap;
    }

    inline void GenInquiryListForEveryDC(vector<ID> &InquiryList, DC dc) {

      for (DC idc = 0; idc < dcnum; idc++) {
        if (idc == dc) continue;
        for (ID iUserNum = 0; iUserNum < userdcnum;   ) {
          InquiryList.push_back(IU[idc][iUserNum].id);
          iUserNum += 2;
        }
      }
    }

    inline void transVisitors(vector<DC> & Visitor, uint64_t VisitBitmap) {

      uint16_t isDC;
      for (int i = 0; i < dcnum; i++) {
        isDC = VisitBitmap & 0x01;
        VisitBitmap >>= 1;
        if (isDC == 1)
          Visitor.push_back(i);
      }
    }

    vector<DC> CalculateRttCost(DC iDC, DC Home, uint64_t VisitBitmap) {

      vector<DC> cost(2, 0);
      vector<DC> Visitor{Home};
      vector<uint64_t> VisitorRttCost;

      transVisitors(Visitor, VisitBitmap);
      for (const auto el: Visitor)
        VisitorRttCost.push_back(dijkstraDC[el][iDC]);

      size_t VisitorNum = Visitor.size();
      DC VisitorContainInfo = rand() % (VisitorNum);

      for (int i = 0; i < VisitorContainInfo + 1; i++) {
        cost[1] += VisitorRttCost[i];
      }
      //
      cost[0] += VisitorRttCost[VisitorContainInfo];

      /*
      for (size_t i = 0; i < VisitorRttCost.size(); i++) {   //{const auto el: VisitorRttCost) {
        auto MinRtt = min_element(begin(VisitorRttCost), end(VisitorRttCost));
        auto MinRttPosition = distance(VisitorRttCost.begin(), MinRtt);

        cost[0] += *MinRtt;
        VisitorRttCost[MinRttPosition] = (1U << 30);
        if (MinRttPosition == VisitorContainInfo)
          break;
      }
      */

      return cost;
    }

    ~LudoNearStateofArt() {
    }

    void test() {
      testExtension();
    }

};

