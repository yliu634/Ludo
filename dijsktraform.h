#include <vector>

void gen_dijkstraDC(vector<vector<uint16_t>> &RTTform, uint16_t dcnum) {
  /*RTTform.resize(dcnum);
  for (auto el:RTTform)
      el = vector<uint32_t>(dcnum, 0);*/
  /*int tmp[dcnum][dcnum] =
          {{0,   35,  109, 50,  272, 251, 259, 276, 181, 288, 152, 185, 175, 188},
           {35,  0,   126, 67,  228, 222, 232, 231, 155, 265, 125, 160, 149, 92},
           {109, 126, 0,   60,  349, 342, 354, 353, 274, 387, 245, 282, 269, 214},
           {50,  67,  60,  0,   297, 284, 294, 309, 215, 326, 186, 222, 210, 157},
           {272, 228, 349, 297, 0,   6,   9,   7,   88,  209, 114, 93,  81,  182},
           {251, 222, 342, 284, 6,   0,   12,  10,  86,  203, 100, 86,  76,  132},
           {259, 232, 354, 294, 9,   12,  0,   7,   92,  214, 110, 98,  86,  143},
           {276, 231, 353, 309, 7,   10,  7,   0,   95,  212, 106, 98,  84,  164},
           {181, 155, 274, 215, 88,  86,  92,  95,  0,   142, 31,  25,  14,  65},
           {288, 265, 387, 326, 209, 203, 214, 212, 142, 0,   144, 117, 129, 171},
           {152, 125, 245, 186, 114, 100, 110, 106, 31,  144, 0,   36,  25,  36},
           {185, 160, 282, 222, 93,  86,  98,  98,  25,  117, 36,  0,   12,  81},
           {175, 149, 169, 210, 81,  76,  86,  84,  14,  129, 25,  12,  0,   58},
           {118, 92,  214, 157, 182, 132, 143, 164, 65,  171, 36,  81,  58,  0}};
  */
/*
  for (int i = 0; i < dcnum; i++) {
    RTTform[i][i] = 0;
    if (i == dcnum - 1)
      continue;
    for (int j = i + 1; j < dcnum; j++) {
      RTTform[i][j] = rand() % 500;
      RTTform[j][i] = RTTform[i][j];
    }
  }
*/

  uint16_t a = ceil(sqrt(dcnum));
  for (int i = 0; i < dcnum; i++) {
    RTTform[i][i] = 0;
    if (i == dcnum - 1)
      continue;
    for (int j = i + 1; j < dcnum; j++) {
      RTTform[i][j] =  50 * (  fabs((i/a)-(j/a)) + fabs((i%a)-(j%a)) );
      RTTform[j][i] = RTTform[i][j];
    }
  }


  //cout << abs((0%a)-(1%a)) <<endl;

  cout << "RTT completed.";
}//end

/*for(auto iter = RTTform.cbegin();iter!=RTTform.cend();iter++){
    for(auto iter2=iter->begin();iter2!=iter->end();iter2++){

    }
}*/

