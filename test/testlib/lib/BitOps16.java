package lib;

/* influnenced by:
 * http://stackoverflow.com/questions/2982729/is-it-possible-to-implement-bitwise-operators-using-integer-arithmetic
 */

/* standard bit operations for numbers 0 .. 32768 (others might work) */

public class BitOps16 {

  /*pu blic static void main(String[] args) {
    /* test for regular java */
    /*
    BitOps16 bo = new BitOps16().init();

    for (int a = 0; a < 32768; a += 101) {
      for (int b = 0; b < 32768; b += 99) {
        if ( bo.i16and(a, b) != (a & b) ) {
          System.out.println("i16and error for " + a + " & " + b);
          return;
        }
        if ( bo.i16or(a, b) != (a | b) ) {
          System.out.println("i16or error for " + a + " | " + b);
          return;
        }
        if ( bo.i16xor(a, b) != (a ^ b) ) {
          System.out.println("i16xor error for " + a + " ^ " + b);
          return;
        }
        if ( bo.i16compl(a) != (~a) ) {
          System.out.println("i16compl error for ~" + a );
          System.out.println("have " + (short)bo.i16compl(a) + " expected " + (short)(~a));
          return;
        }
      }
    }
    System.out.println("All and/or/xor/compl ok for < 65536!");
    for (int a = 0; a < 65536; a += 1) { // seem to be off by one or negated from -32768 to 0
      for (int b = 0; b < 15; b += 1) { // only to 15 since 16 bit op
        if ( (short)bo.i16shl(a, b) != (short)(a << b) ) {
          System.out.println("i16shl error for " + a + " << " + b);
          System.out.println("have " + (short)bo.i16shl(a,b) + " expected " + (short)(a << b));
          return;
        }
        if ( (short)bo.ui16shr(a,b) != (short)(a >>> b) ) {
          System.out.println("ui16shr error for " + a + " >>> " + b);
          System.out.println("have " + (short)bo.ui16shr(a,b) + " expected " + (short)(a >>> b));
          return;
        }
        if ( (short)bo.si16shr(a, b) != (short)(a >> b) ) {
          System.out.println("si16shr error for " + a + " >> " + b);
          System.out.println("have " + (short)bo.si16shr(a,b) + " expected " + (short)(a >> b));
          return;
        }
      }
    }
    System.out.println("All shifts ok for 0 <= x < 65536 (is -32768 <= x < 32768 for int16)!");

  }*/

  public int[] powtab;
  public int[] andlookup;

  /* logical shift left */
  public int i16shl(int i, int shift) {
    if (shift > 15) {
        return 0; /* if shifting more than 15 bits to the left, value is always zero */
    } else {
        return i * powtab[shift] % 65536;
    }
  }

  public int ui16shl(int i, int shift) {
    if (shift > 15) {
      return 0; /* if shifting more than 15 bits to the left, value is always zero */
    } else if (shift > 0){
      return i * powtab[shift] % 65536;
    } else {
      return i;
    }
  }

  /* logical shift right (unsigned) */
  public int ui16shr(int i, int shift) {
    if (shift > 15) {
      return 0; /* more than 15, becomes zero */
    } else if (shift > 0) {
      return i / powtab[shift];
    } else {
      return i; /* no shift */
    }
  }

  /* arithmetic shift right (signed) */
  public int si16shr(int i, int shift) {
    if (shift >= 15) {
      if (i < 0) {
          return -1;
      } else {
          return 0;
      }
    } else if (shift > 0) {
      if (i < 0) {
        /* deal with the sign bit */
        i = i + -2147483648;
        i = i / powtab[shift];
        return i + -2147483648;
      } else {
        /* same as unsigned shift */
        return i / powtab[shift];
      }
    } else {
      return i; /* no shift */
    }
  }

  public int ui16rotl(int i, int rot)
  {
    rot = rot % 16;
    return i16or(ui16shl(i, rot), ui16shr(i, (16 - rot)));
  }

  public int ui16rotr(int i, int rot)
  {
    rot = rot % 16;
    return i16or(ui16shr(i, rot), ui16shl(i, (16 - rot)));
  }

  public int i16compl(int i) {
    return (-1 - i) % 65536;
  }

  public int i16and(int a, int b) {
    int r = 0;
    int i = 0;

    while (i < 16) {
      r = r/16 + andlookup[(a%16)*16+(b%16)]*4096;
      a = a / 16;
      b = b / 16;
      i = i + 4; /* loop counter */
    }
    return r;
  }

  public int i16or(int i1, int i2) {
    return ((i1 + i2 - 2*i16and(i1,i2)) + i16and(i1,i2) - 2*i16and((i1 + i2 - 2*i16and(i1,i2)),i16and(i1,i2)));
  }

  public int i16xor(int i1, int i2) {
    return (i1 + i2 - 2*i16and(i1,i2));
  }

  public BitOps16 init() {
    powtab = getPowLookupTable();
    andlookup = getAndLookupTable();
    return this;
  }

  public int[] getPowLookupTable() {
    /* { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, -32768 }; */
    int[] pt = new int[16];
    pt[0] = 1;
    pt[1] = 2;
    pt[2] = 4;
    pt[3] = 8;
    pt[4] = 16;
    pt[5] = 32;
    pt[6] = 64;
    pt[7] = 128;
    pt[8] = 256;
    pt[9] = 512;
    pt[10] = 1024;
    pt[11] = 2048;
    pt[12] = 4096;
    pt[13] = 8192;
    pt[14] = 16384;
    pt[15] = 32768;
    return pt;
  }

  public int[] getAndLookupTable() {
    int[] al = new int[256];
    al[0] = 0;
    al[1] = 0;
    al[2] = 0;
    al[3] = 0;
    al[4] = 0;
    al[5] = 0;
    al[6] = 0;
    al[7] = 0;
    al[8] = 0;
    al[9] = 0;
    al[10] = 0;
    al[11] = 0;
    al[12] = 0;
    al[13] = 0;
    al[14] = 0;
    al[15] = 0;
    al[16] = 0;
    al[17] = 1;
    al[18] = 0;
    al[19] = 1;
    al[20] = 0;
    al[21] = 1;
    al[22] = 0;
    al[23] = 1;
    al[24] = 0;
    al[25] = 1;
    al[26] = 0;
    al[27] = 1;
    al[28] = 0;
    al[29] = 1;
    al[30] = 0;
    al[31] = 1;
    al[32] = 0;
    al[33] = 0;
    al[34] = 2;
    al[35] = 2;
    al[36] = 0;
    al[37] = 0;
    al[38] = 2;
    al[39] = 2;
    al[40] = 0;
    al[41] = 0;
    al[42] = 2;
    al[43] = 2;
    al[44] = 0;
    al[45] = 0;
    al[46] = 2;
    al[47] = 2;
    al[48] = 0;
    al[49] = 1;
    al[50] = 2;
    al[51] = 3;
    al[52] = 0;
    al[53] = 1;
    al[54] = 2;
    al[55] = 3;
    al[56] = 0;
    al[57] = 1;
    al[58] = 2;
    al[59] = 3;
    al[60] = 0;
    al[61] = 1;
    al[62] = 2;
    al[63] = 3;
    al[64] = 0;
    al[65] = 0;
    al[66] = 0;
    al[67] = 0;
    al[68] = 4;
    al[69] = 4;
    al[70] = 4;
    al[71] = 4;
    al[72] = 0;
    al[73] = 0;
    al[74] = 0;
    al[75] = 0;
    al[76] = 4;
    al[77] = 4;
    al[78] = 4;
    al[79] = 4;
    al[80] = 0;
    al[81] = 1;
    al[82] = 0;
    al[83] = 1;
    al[84] = 4;
    al[85] = 5;
    al[86] = 4;
    al[87] = 5;
    al[88] = 0;
    al[89] = 1;
    al[90] = 0;
    al[91] = 1;
    al[92] = 4;
    al[93] = 5;
    al[94] = 4;
    al[95] = 5;
    al[96] = 0;
    al[97] = 0;
    al[98] = 2;
    al[99] = 2;
    al[100] = 4;
    al[101] = 4;
    al[102] = 6;
    al[103] = 6;
    al[104] = 0;
    al[105] = 0;
    al[106] = 2;
    al[107] = 2;
    al[108] = 4;
    al[109] = 4;
    al[110] = 6;
    al[111] = 6;
    al[112] = 0;
    al[113] = 1;
    al[114] = 2;
    al[115] = 3;
    al[116] = 4;
    al[117] = 5;
    al[118] = 6;
    al[119] = 7;
    al[120] = 0;
    al[121] = 1;
    al[122] = 2;
    al[123] = 3;
    al[124] = 4;
    al[125] = 5;
    al[126] = 6;
    al[127] = 7;
    al[128] = 0;
    al[129] = 0;
    al[130] = 0;
    al[131] = 0;
    al[132] = 0;
    al[133] = 0;
    al[134] = 0;
    al[135] = 0;
    al[136] = 8;
    al[137] = 8;
    al[138] = 8;
    al[139] = 8;
    al[140] = 8;
    al[141] = 8;
    al[142] = 8;
    al[143] = 8;
    al[144] = 0;
    al[145] = 1;
    al[146] = 0;
    al[147] = 1;
    al[148] = 0;
    al[149] = 1;
    al[150] = 0;
    al[151] = 1;
    al[152] = 8;
    al[153] = 9;
    al[154] = 8;
    al[155] = 9;
    al[156] = 8;
    al[157] = 9;
    al[158] = 8;
    al[159] = 9;
    al[160] = 0;
    al[161] = 0;
    al[162] = 2;
    al[163] = 2;
    al[164] = 0;
    al[165] = 0;
    al[166] = 2;
    al[167] = 2;
    al[168] = 8;
    al[169] = 8;
    al[170] = 10;
    al[171] = 10;
    al[172] = 8;
    al[173] = 8;
    al[174] = 10;
    al[175] = 10;
    al[176] = 0;
    al[177] = 1;
    al[178] = 2;
    al[179] = 3;
    al[180] = 0;
    al[181] = 1;
    al[182] = 2;
    al[183] = 3;
    al[184] = 8;
    al[185] = 9;
    al[186] = 10;
    al[187] = 11;
    al[188] = 8;
    al[189] = 9;
    al[190] = 10;
    al[191] = 11;
    al[192] = 0;
    al[193] = 0;
    al[194] = 0;
    al[195] = 0;
    al[196] = 4;
    al[197] = 4;
    al[198] = 4;
    al[199] = 4;
    al[200] = 8;
    al[201] = 8;
    al[202] = 8;
    al[203] = 8;
    al[204] = 12;
    al[205] = 12;
    al[206] = 12;
    al[207] = 12;
    al[208] = 0;
    al[209] = 1;
    al[210] = 0;
    al[211] = 1;
    al[212] = 4;
    al[213] = 5;
    al[214] = 4;
    al[215] = 5;
    al[216] = 8;
    al[217] = 9;
    al[218] = 8;
    al[219] = 9;
    al[220] = 12;
    al[221] = 13;
    al[222] = 12;
    al[223] = 13;
    al[224] = 0;
    al[225] = 0;
    al[226] = 2;
    al[227] = 2;
    al[228] = 4;
    al[229] = 4;
    al[230] = 6;
    al[231] = 6;
    al[232] = 8;
    al[233] = 8;
    al[234] = 10;
    al[235] = 10;
    al[236] = 12;
    al[237] = 12;
    al[238] = 14;
    al[239] = 14;
    al[240] = 0;
    al[241] = 1;
    al[242] = 2;
    al[243] = 3;
    al[244] = 4;
    al[245] = 5;
    al[246] = 6;
    al[247] = 7;
    al[248] = 8;
    al[249] = 9;
    al[250] = 10;
    al[251] = 11;
    al[252] = 12;
    al[253] = 13;
    al[254] = 14;
    al[255] = 15;
    return al;
  }
}
