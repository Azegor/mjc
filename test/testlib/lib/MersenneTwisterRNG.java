package lib;

/**
 * Mersenne Twister generator for pseudo random numbers.
 * adapted from http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/VERSIONS/JAVA/MTRandom.java
 */
/*
 * MersenneTwisterRNG : A Java implementation of the MT19937 (Mersenne Twister)
 *            pseudo random number generator algorithm based upon the
 *            original C code by Makoto Matsumoto and Takuji Nishimura.
 * Author   : David Beaumont
 * Email    : mersenne-at-www.goui.net
 *
 * For the original C code, see:
 *     http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 *
 * This version, Copyright (C) 2005, David Beaumont.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License aint with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

import lib.BitOps32;

/**
 * @version 1.0
 * @author David Beaumont, Copyright 2005
 */

public class MersenneTwisterRNG {

	public BitOps32 bops;

	public int UPPER_MASK;
	public int LOWER_MASK;

	public int N;
	public int M;
	public int[] MAGIC;
	public int MAGIC_FACTOR1;
	public int MAGIC_FACTOR2;
	public int MAGIC_FACTOR3;
	public int MAGIC_MASK1;
	public int MAGIC_MASK2;
	public int MAGIC_SEED;
	public int DEFAULT_SEED;

	/* Internal state */
	public int[] mt;
	public int mti;

	/* Temporary buffer used during setSeed(int) */
	public int[] ibuf;

	public MersenneTwisterRNG initInternal() {
      bops = new BitOps32().init();

      UPPER_MASK = -2147483648/*0x80000000*/;
      LOWER_MASK = 2147483647/*0x7fffffff*/;
      N = 624;
      M = 397;
      MAGIC = new int[2];
      MAGIC[0] = 0; /*0x0*/
      MAGIC[1] = -1727483681; /*0x9908b0df*/
      MAGIC_FACTOR1 = 1812433253;
      MAGIC_FACTOR2 = 1664525;
      MAGIC_FACTOR3 = 1566083941;
      MAGIC_MASK1   = -1658038656/*0x9d2c5680*/;
      MAGIC_MASK2   = -272236544/*0xefc60000*/;
      MAGIC_SEED    = 19650218;
      DEFAULT_SEED = 5489;
      return this;
	}

    public MersenneTwisterRNG init() {
		initInternal();
        setSeed(DEFAULT_SEED);
		return this;
	}
	public MersenneTwisterRNG initSeed(int seed) {
		initInternal();
        setSeed(seed);
		return this;
	}

	/* TODO
	public MersenneTwisterRNG(int seed) {
		super(seed);
	}

	public MersenneTwisterRNG(int[] buf) {
		super(0L);
		setSeed(buf);
	}

	public MersenneTwisterRNG(int[] buf) {
		super(0L);
		setSeed(buf);
	}
	*/
	public void setSeed(int seed) {
		if (mt == null) mt = new int[N];

		/* ---- Begin Mersenne Twister Algorithm ---- */
		mt[0] = seed;
		int mti = 1;
		while (mti < N) {
			/* mt[mti] = (MAGIC_FACTOR1 * (mt[mti-1] ^ (mt[mti-1] >>> 30)) + mti);*/
			mt[mti] = (MAGIC_FACTOR1 * bops.i32xor(mt[mti-1], bops.ui32shr(mt[mti-1], 30)) + mti);
			mti = mti + 1;
		}
		/* ---- End Mersenne Twister Algorithm ---- */
	}

    /**
	 * This method forms the basis for generating a pseudo random number
	 * sequence from this class.  If given a value of 32, this method
	 * behaves identically to the genrand_int32 function in the original
	 * C code and ensures that using the standard nextInt() function
	 * (inherited from Random) we are able to replicate behaviour exactly.
	 * <p>
	 * Note that where the number of bits requested is not equal to 32
	 * then bits will simply be masked out from the top of the returned
	 * integer value.  That is to say that:
	 * <pre>
	 * mt.setSeed(12345);
	 * int foo = mt.nextInt(16) + (mt.nextInt(16) << 16);</pre>
	 * will not give the same result as
	 * <pre>
	 * mt.setSeed(12345);
	 * int foo = mt.nextInt(32);</pre>
	 *
	 * @param bits The number of significant bits desired in the output.
	 * @return The next value in the pseudo random sequence with the
	 * specified number of bits in the lower part of the integer.
	 */
	public int next(int bits) {
		/* ---- Begin Mersenne Twister Algorithm ----*/
		int y;
		int kk;
		if (mti >= N) {             /* generate N words at one time */
            kk = 0;
			while (kk < N-M) {
				y = bops.i32or(bops.i32and(mt[kk], UPPER_MASK), bops.i32and(mt[kk+1], LOWER_MASK));
				mt[kk] = bops.i32xor(bops.i32xor(mt[kk+M], bops.ui32shr(y, 1)), MAGIC[bops.i32and(y,1)]);
				kk = kk + 1;
			}
			while (kk < N-1) {
				y = bops.i32or(bops.i32and(mt[kk], UPPER_MASK), bops.i32and(mt[kk+1], LOWER_MASK));
				mt[kk] = bops.i32xor(bops.i32xor(mt[kk+(M-N)], bops.ui32shr(y, 1)), MAGIC[bops.i32and(y, 1)]);
				kk = kk + 1;
			}
			y = bops.i32or(bops.i32and(mt[N-1], UPPER_MASK), bops.i32and(mt[0], LOWER_MASK));
			mt[N-1] = bops.i32xor(bops.i32xor(mt[M-1], bops.ui32shr(y, 1)), MAGIC[bops.i32and(y, 1)]);

			mti = 0;
		}

		y = mt[mti];
		mti = mti + 1;

		/* Tempering */
		y = bops.i32xor(y, bops.ui32shr(y, 11));
		y = bops.i32xor(y, bops.i32and(bops.i32shl(y, 7), MAGIC_MASK1));
		y = bops.i32xor(y, bops.i32and(bops.i32shl(y, 15), MAGIC_MASK2));
		y = bops.i32xor(y, bops.ui32shr(y, 18));
		/* ---- End Mersenne Twister Algorithm ---- */
		return bops.ui32shr(y, (32-bits));
	}

    /**
	 * This simply utility method can be used in cases where a byte
	 * array of seed data is to be used to repeatedly re-seed the
	 * random number sequence.  By packing the byte array into an
	 * integer array first, using this method, and then invoking
	 * setSeed() with that; it removes the need to re-pack the byte
	 * array each time setSeed() is called.
	 * <p>
	 * If the length of the byte array is not a multiple of 4 then
	 * it is implicitly padded with zeros as necessary.  For example:
	 * <pre>    byte[] { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 }</pre>
	 * becomes
	 * <pre>    int[]  { 0x04030201, 0x00000605 }</pre>
	 * <p>
	 * Note that this method will not complain if the given byte array
	 * is empty and will produce an empty integer array, but the
	 * setSeed() method will throw an exception if the empty integer
	 * array is passed to it.
	 *
	 * @param buf The non-null byte array to be packed.
	 * @return A non-null integer array of the packed bytes.
	 * @throws NullPointerException if the given byte array is null.
	 */
	public int[] pack(int[] buf, int length) {
		int k;
		int blen = length;
		int ilen = bops.ui32shr((length+3), 2);
		int[] ibuf = new int[ilen];
		int n = 0;
		while (n < ilen) {
			int m = bops.i32shl((n+1), 2);
			if (m > blen) m = blen;
			m = m - 1;
			k = bops.i32and(buf[m], 255);
			while (bops.i32and(m, 3) != 0);
			ibuf[n] = k;
			n = n + 1;
			m = m - 1;
			k = bops.i32or(bops.i32shl(k, 8), bops.i32and(buf[m], 255));
		}
		return ibuf;
	}
}
