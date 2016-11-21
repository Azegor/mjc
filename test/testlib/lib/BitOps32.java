package lib;

/* influnenced by:
 * http://stackoverflow.com/questions/2982729/is-it-possible-to-implement-bitwise-operators-using-integer-arithmetic
 */

/* standard bit operations for numbers 0 .. 2^31 (others might work) */

public class BitOps32 {

  /* test for regular java */
  /*
  public static void main(String[] args) {
    BitOps32 bo = new BitOps32().init();

    for (int a = -2147483648; a < 2147483647 - 1010101; a += 1010101) {
      for (int b = -2147483648; b < 2147483647 - 9090909; b += 2090909) {
        if ( bo.i32and(a, b) != (a & b) ) {
          System.out.println("i32and error for " + a + " & " + b);
          System.out.println("have " + bo.i32and(a, b) + " expected " + (a & b));
          return;
        }
        if ( bo.i32or(a, b) != (a | b) ) {
          System.out.println("i32or error for " + a + " | " + b);
          System.out.println("have " + bo.i32or(a, b) + " expected " + (a | b));
          return;
        }
        if ( bo.i32xor(a, b) != (a ^ b) ) {
          System.out.println("i32xor error for " + a + " ^ " + b);
          System.out.println("have " + bo.i32xor(a, b) + " expected " + (a ^ b));
          return;
        }
      }
    }
    System.out.println("All and/or/xor ok for -2147483648 <= x < 2147483647!");
    for (int a = -1; a < 2147483647 - 99; a += 999) { // seem to be off by one or negated from -32768 to 0
      for (int b = 0; b < 31; b += 1) { // only to 15, since java uses 32 bit
        if ( bo.i32shl(a, b) != (a << b) ) {
          System.out.println("i32shl error for " + a + " << " + b);
          System.out.println("have " + bo.i32shl(a,b) + " expected " + (a << b));
          return;
        }
        if ( bo.ui32shr(a,b) != (a >>> b) ) {
          System.out.println("ui32shr error for " + a + " >>> " + b);
          System.out.println("have " + bo.ui32shr(a,b) + " expected " + (a >>> b));
          return;
        }
        if ( bo.si32shr(a, b) != (a >> b) ) {
          System.out.println("si32shr error for " + a + " >> " + b);
          System.out.println("have " + bo.si32shr(a,b) + " expected " + (a >> b));
          return;
        }
      }
    }
    System.out.println("All shifts ok for 0 <= x < 2147483647!");
  }
  */

  public int[] powtab;
  public int[] shiftMultiplies;
  public int[] andlookup;

  /* logical shift left */
  public int i32shl(int i, int shift) {
    if (shift > 31) {
        return 0; /* if shifting more than 15 bits to the left, value is always zero */
    } else {
        return i * powtab[shift];
    }
  }

  /* logical shift right (unsigned) */
  public int ui32shr(int i, int shift) {
    if (shift > 31) {
        return 0; /* more than 15, becomes zero */
    } else if (shift > 0) {
      if (i < 0) {
          /* deal with the sign bit (15) */
          i = i + -2147483648;
          i = i / powtab[shift];
          return i + powtab[31 - shift];
      } else {
          return i / powtab[shift];
      }
    } else {
      return i; /* no shift */
    }
  }

  /* arithmetic shift right (signed) */
  public int si32shr(int i, int shift) {
    if (shift >= 31) {
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
          return i - powtab[31 - shift];
      } else {
          /* same as unsigned shift */
          return i / powtab[shift];
      }
    } else {
      return i; /* no shift */
    }
  }

  public int i32compl(int i) {
    return -1 - i;
  }

  public int ui32rotl(int i, int rot)
  {
    rot = rot % 32;
    return i32or(i32shl(i, rot), ui32shr(i, (32 - rot)));
  }

  public int ui32rotr(int i, int rot)
  {
    rot = rot % 32;
    return i32or(ui32shr(i, rot), i32shl(i, (32 - rot)));
  }

  public int i32and(int a, int b) {
    int r = 0;
    int i = 0;

    boolean signBit = false;;
    if (a < 0) {
      signBit = true;
      a = a + -2147483648; /* remove sign bit */
    }
    if (b < 0) {
      /* keep signBit's value ( = signBit && true )*/
      b = b + -2147483648; /* remove sign bit */
    } else {
      signBit = false; /* = signBit && false; */
    }

    while (i < 8) {
      /* do it different than in 32 bit variant since sign bit creates problems otherwise */
      r = r + andlookup[(a%16)*16+(b%16)] * shiftMultiplies[i];
      a = a / 16;
      b = b / 16;
      i = i + 1; /* loop counter */
    }

     /* signBit already 'and'ed */
    if (signBit) {
      r = r + -2147483648; /* readd signbit only if both in input where set */
    }
    return r;
  }

  public int i32or(int i1, int i2) {
    return ((i1 + i2 - 2*i32and(i1,i2)) + i32and(i1,i2) - 2*i32and((i1 + i2 - 2*i32and(i1,i2)),i32and(i1,i2)));
  }

  public int i32xor(int i1, int i2) {
    return (i1 + i2 - 2*i32and(i1,i2));
  }

  public BitOps32 init() {
    powtab = getPowLookupTable();
    shiftMultiplies = getShiftMultipliers();
    andlookup = getAndLookupTable();
    return this;
  }

  public int[] getPowLookupTable() {
    int[] pt = new int[32];
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
    pt[16] = 65536;
    pt[17] = 131072;
    pt[18] = 262144;
    pt[19] = 524288;
    pt[20] = 1048576;
    pt[21] = 2097152;
    pt[22] = 4194304;
    pt[23] = 8388608;
    pt[24] = 16777216;
    pt[25] = 33554432;
    pt[26] = 67108864;
    pt[27] = 134217728;
    pt[28] = 268435456;
    pt[29] = 536870912;
    pt[30] = 1073741824;
    pt[31] = -2147483648;
    return pt;
  }
  public int[] getShiftMultipliers() {
    int[] sm = new int[8];
    sm[0] = 1;
    sm[1] = 16;
    sm[2] = 256;
    sm[3] = 4096;
    sm[4] = 65536;
    sm[5] = 1048576;
    sm[6] = 16777216;
    sm[7] = 268435456;
    return sm;
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
