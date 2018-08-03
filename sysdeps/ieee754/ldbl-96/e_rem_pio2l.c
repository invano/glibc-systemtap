/* Extended-precision floating point argument reduction.
   Copyright (C) 1999-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on quad-precision code by Jakub Jelinek <jj@ultra.linux.cz>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <math.h>
#include <math_private.h>

/*  Table of constants for 2/pi, 5628 hexadecimal digits of 2/pi.  */
static const int32_t two_over_pi[] = {
0xa2f983, 0x6e4e44, 0x1529fc, 0x2757d1, 0xf534dd, 0xc0db62,
0x95993c, 0x439041, 0xfe5163, 0xabdebb, 0xc561b7, 0x246e3a,
0x424dd2, 0xe00649, 0x2eea09, 0xd1921c, 0xfe1deb, 0x1cb129,
0xa73ee8, 0x8235f5, 0x2ebb44, 0x84e99c, 0x7026b4, 0x5f7e41,
0x3991d6, 0x398353, 0x39f49c, 0x845f8b, 0xbdf928, 0x3b1ff8,
0x97ffde, 0x05980f, 0xef2f11, 0x8b5a0a, 0x6d1f6d, 0x367ecf,
0x27cb09, 0xb74f46, 0x3f669e, 0x5fea2d, 0x7527ba, 0xc7ebe5,
0xf17b3d, 0x0739f7, 0x8a5292, 0xea6bfb, 0x5fb11f, 0x8d5d08,
0x560330, 0x46fc7b, 0x6babf0, 0xcfbc20, 0x9af436, 0x1da9e3,
0x91615e, 0xe61b08, 0x659985, 0x5f14a0, 0x68408d, 0xffd880,
0x4d7327, 0x310606, 0x1556ca, 0x73a8c9, 0x60e27b, 0xc08c6b,
0x47c419, 0xc367cd, 0xdce809, 0x2a8359, 0xc4768b, 0x961ca6,
0xddaf44, 0xd15719, 0x053ea5, 0xff0705, 0x3f7e33, 0xe832c2,
0xde4f98, 0x327dbb, 0xc33d26, 0xef6b1e, 0x5ef89f, 0x3a1f35,
0xcaf27f, 0x1d87f1, 0x21907c, 0x7c246a, 0xfa6ed5, 0x772d30,
0x433b15, 0xc614b5, 0x9d19c3, 0xc2c4ad, 0x414d2c, 0x5d000c,
0x467d86, 0x2d71e3, 0x9ac69b, 0x006233, 0x7cd2b4, 0x97a7b4,
0xd55537, 0xf63ed7, 0x1810a3, 0xfc764d, 0x2a9d64, 0xabd770,
0xf87c63, 0x57b07a, 0xe71517, 0x5649c0, 0xd9d63b, 0x3884a7,
0xcb2324, 0x778ad6, 0x23545a, 0xb91f00, 0x1b0af1, 0xdfce19,
0xff319f, 0x6a1e66, 0x615799, 0x47fbac, 0xd87f7e, 0xb76522,
0x89e832, 0x60bfe6, 0xcdc4ef, 0x09366c, 0xd43f5d, 0xd7de16,
0xde3b58, 0x929bde, 0x2822d2, 0xe88628, 0x4d58e2, 0x32cac6,
0x16e308, 0xcb7de0, 0x50c017, 0xa71df3, 0x5be018, 0x34132e,
0x621283, 0x014883, 0x5b8ef5, 0x7fb0ad, 0xf2e91e, 0x434a48,
0xd36710, 0xd8ddaa, 0x425fae, 0xce616a, 0xa4280a, 0xb499d3,
0xf2a606, 0x7f775c, 0x83c2a3, 0x883c61, 0x78738a, 0x5a8caf,
0xbdd76f, 0x63a62d, 0xcbbff4, 0xef818d, 0x67c126, 0x45ca55,
0x36d9ca, 0xd2a828, 0x8d61c2, 0x77c912, 0x142604, 0x9b4612,
0xc459c4, 0x44c5c8, 0x91b24d, 0xf31700, 0xad43d4, 0xe54929,
0x10d5fd, 0xfcbe00, 0xcc941e, 0xeece70, 0xf53e13, 0x80f1ec,
0xc3e7b3, 0x28f8c7, 0x940593, 0x3e71c1, 0xb3092e, 0xf3450b,
0x9c1288, 0x7b20ab, 0x9fb52e, 0xc29247, 0x2f327b, 0x6d550c,
0x90a772, 0x1fe76b, 0x96cb31, 0x4a1679, 0xe27941, 0x89dff4,
0x9794e8, 0x84e6e2, 0x973199, 0x6bed88, 0x365f5f, 0x0efdbb,
0xb49a48, 0x6ca467, 0x427271, 0x325d8d, 0xb8159f, 0x09e5bc,
0x25318d, 0x3974f7, 0x1c0530, 0x010c0d, 0x68084b, 0x58ee2c,
0x90aa47, 0x02e774, 0x24d6bd, 0xa67df7, 0x72486e, 0xef169f,
0xa6948e, 0xf691b4, 0x5153d1, 0xf20acf, 0x339820, 0x7e4bf5,
0x6863b2, 0x5f3edd, 0x035d40, 0x7f8985, 0x295255, 0xc06437,
0x10d86d, 0x324832, 0x754c5b, 0xd4714e, 0x6e5445, 0xc1090b,
0x69f52a, 0xd56614, 0x9d0727, 0x50045d, 0xdb3bb4, 0xc576ea,
0x17f987, 0x7d6b49, 0xba271d, 0x296996, 0xacccc6, 0x5414ad,
0x6ae290, 0x89d988, 0x50722c, 0xbea404, 0x940777, 0x7030f3,
0x27fc00, 0xa871ea, 0x49c266, 0x3de064, 0x83dd97, 0x973fa3,
0xfd9443, 0x8c860d, 0xde4131, 0x9d3992, 0x8c70dd, 0xe7b717,
0x3bdf08, 0x2b3715, 0xa0805c, 0x93805a, 0x921110, 0xd8e80f,
0xaf806c, 0x4bffdb, 0x0f9038, 0x761859, 0x15a562, 0xbbcb61,
0xb989c7, 0xbd4010, 0x04f2d2, 0x277549, 0xf6b6eb, 0xbb22db,
0xaa140a, 0x2f2689, 0x768364, 0x333b09, 0x1a940e, 0xaa3a51,
0xc2a31d, 0xaeedaf, 0x12265c, 0x4dc26d, 0x9c7a2d, 0x9756c0,
0x833f03, 0xf6f009, 0x8c402b, 0x99316d, 0x07b439, 0x15200c,
0x5bc3d8, 0xc492f5, 0x4badc6, 0xa5ca4e, 0xcd37a7, 0x36a9e6,
0x9492ab, 0x6842dd, 0xde6319, 0xef8c76, 0x528b68, 0x37dbfc,
0xaba1ae, 0x3115df, 0xa1ae00, 0xdafb0c, 0x664d64, 0xb705ed,
0x306529, 0xbf5657, 0x3aff47, 0xb9f96a, 0xf3be75, 0xdf9328,
0x3080ab, 0xf68c66, 0x15cb04, 0x0622fa, 0x1de4d9, 0xa4b33d,
0x8f1b57, 0x09cd36, 0xe9424e, 0xa4be13, 0xb52333, 0x1aaaf0,
0xa8654f, 0xa5c1d2, 0x0f3f0b, 0xcd785b, 0x76f923, 0x048b7b,
0x721789, 0x53a6c6, 0xe26e6f, 0x00ebef, 0x584a9b, 0xb7dac4,
0xba66aa, 0xcfcf76, 0x1d02d1, 0x2df1b1, 0xc1998c, 0x77adc3,
0xda4886, 0xa05df7, 0xf480c6, 0x2ff0ac, 0x9aecdd, 0xbc5c3f,
0x6dded0, 0x1fc790, 0xb6db2a, 0x3a25a3, 0x9aaf00, 0x9353ad,
0x0457b6, 0xb42d29, 0x7e804b, 0xa707da, 0x0eaa76, 0xa1597b,
0x2a1216, 0x2db7dc, 0xfde5fa, 0xfedb89, 0xfdbe89, 0x6c76e4,
0xfca906, 0x70803e, 0x156e85, 0xff87fd, 0x073e28, 0x336761,
0x86182a, 0xeabd4d, 0xafe7b3, 0x6e6d8f, 0x396795, 0x5bbf31,
0x48d784, 0x16df30, 0x432dc7, 0x356125, 0xce70c9, 0xb8cb30,
0xfd6cbf, 0xa200a4, 0xe46c05, 0xa0dd5a, 0x476f21, 0xd21262,
0x845cb9, 0x496170, 0xe0566b, 0x015299, 0x375550, 0xb7d51e,
0xc4f133, 0x5f6e13, 0xe4305d, 0xa92e85, 0xc3b21d, 0x3632a1,
0xa4b708, 0xd4b1ea, 0x21f716, 0xe4698f, 0x77ff27, 0x80030c,
0x2d408d, 0xa0cd4f, 0x99a520, 0xd3a2b3, 0x0a5d2f, 0x42f9b4,
0xcbda11, 0xd0be7d, 0xc1db9b, 0xbd17ab, 0x81a2ca, 0x5c6a08,
0x17552e, 0x550027, 0xf0147f, 0x8607e1, 0x640b14, 0x8d4196,
0xdebe87, 0x2afdda, 0xb6256b, 0x34897b, 0xfef305, 0x9ebfb9,
0x4f6a68, 0xa82a4a, 0x5ac44f, 0xbcf82d, 0x985ad7, 0x95c7f4,
0x8d4d0d, 0xa63a20, 0x5f57a4, 0xb13f14, 0x953880, 0x0120cc,
0x86dd71, 0xb6dec9, 0xf560bf, 0x11654d, 0x6b0701, 0xacb08c,
0xd0c0b2, 0x485551, 0x0efb1e, 0xc37295, 0x3b06a3, 0x3540c0,
0x7bdc06, 0xcc45e0, 0xfa294e, 0xc8cad6, 0x41f3e8, 0xde647c,
0xd8649b, 0x31bed9, 0xc397a4, 0xd45877, 0xc5e369, 0x13daf0,
0x3c3aba, 0x461846, 0x5f7555, 0xf5bdd2, 0xc6926e, 0x5d2eac,
0xed440e, 0x423e1c, 0x87c461, 0xe9fd29, 0xf3d6e7, 0xca7c22,
0x35916f, 0xc5e008, 0x8dd7ff, 0xe26a6e, 0xc6fdb0, 0xc10893,
0x745d7c, 0xb2ad6b, 0x9d6ecd, 0x7b723e, 0x6a11c6, 0xa9cff7,
0xdf7329, 0xbac9b5, 0x5100b7, 0x0db2e2, 0x24ba74, 0x607de5,
0x8ad874, 0x2c150d, 0x0c1881, 0x94667e, 0x162901, 0x767a9f,
0xbefdfd, 0xef4556, 0x367ed9, 0x13d9ec, 0xb9ba8b, 0xfc97c4,
0x27a831, 0xc36ef1, 0x36c594, 0x56a8d8, 0xb5a8b4, 0x0ecccf,
0x2d8912, 0x34576f, 0x89562c, 0xe3ce99, 0xb920d6, 0xaa5e6b,
0x9c2a3e, 0xcc5f11, 0x4a0bfd, 0xfbf4e1, 0x6d3b8e, 0x2c86e2,
0x84d4e9, 0xa9b4fc, 0xd1eeef, 0xc9352e, 0x61392f, 0x442138,
0xc8d91b, 0x0afc81, 0x6a4afb, 0xd81c2f, 0x84b453, 0x8c994e,
0xcc2254, 0xdc552a, 0xd6c6c0, 0x96190b, 0xb8701a, 0x649569,
0x605a26, 0xee523f, 0x0f117f, 0x11b5f4, 0xf5cbfc, 0x2dbc34,
0xeebc34, 0xcc5de8, 0x605edd, 0x9b8e67, 0xef3392, 0xb817c9,
0x9b5861, 0xbc57e1, 0xc68351, 0x103ed8, 0x4871dd, 0xdd1c2d,
0xa118af, 0x462c21, 0xd7f359, 0x987ad9, 0xc0549e, 0xfa864f,
0xfc0656, 0xae79e5, 0x362289, 0x22ad38, 0xdc9367, 0xaae855,
0x382682, 0x9be7ca, 0xa40d51, 0xb13399, 0x0ed7a9, 0x480569,
0xf0b265, 0xa7887f, 0x974c88, 0x36d1f9, 0xb39221, 0x4a827b,
0x21cf98, 0xdc9f40, 0x5547dc, 0x3a74e1, 0x42eb67, 0xdf9dfe,
0x5fd45e, 0xa4677b, 0x7aacba, 0xa2f655, 0x23882b, 0x55ba41,
0x086e59, 0x862a21, 0x834739, 0xe6e389, 0xd49ee5, 0x40fb49,
0xe956ff, 0xca0f1c, 0x8a59c5, 0x2bfa94, 0xc5c1d3, 0xcfc50f,
0xae5adb, 0x86c547, 0x624385, 0x3b8621, 0x94792c, 0x876110,
0x7b4c2a, 0x1a2c80, 0x12bf43, 0x902688, 0x893c78, 0xe4c4a8,
0x7bdbe5, 0xc23ac4, 0xeaf426, 0x8a67f7, 0xbf920d, 0x2ba365,
0xb1933d, 0x0b7cbd, 0xdc51a4, 0x63dd27, 0xdde169, 0x19949a,
0x9529a8, 0x28ce68, 0xb4ed09, 0x209f44, 0xca984e, 0x638270,
0x237c7e, 0x32b90f, 0x8ef5a7, 0xe75614, 0x08f121, 0x2a9db5,
0x4d7e6f, 0x5119a5, 0xabf9b5, 0xd6df82, 0x61dd96, 0x023616,
0x9f3ac4, 0xa1a283, 0x6ded72, 0x7a8d39, 0xa9b882, 0x5c326b,
0x5b2746, 0xed3400, 0x7700d2, 0x55f4fc, 0x4d5901, 0x8071e0,
0xe13f89, 0xb295f3, 0x64a8f1, 0xaea74b, 0x38fc4c, 0xeab2bb,
0x47270b, 0xabc3a7, 0x34ba60, 0x52dd34, 0xf8563a, 0xeb7e8a,
0x31bb36, 0x5895b7, 0x47f7a9, 0x94c3aa, 0xd39225, 0x1e7f3e,
0xd8974e, 0xbba94f, 0xd8ae01, 0xe661b4, 0x393d8e, 0xa523aa,
0x33068e, 0x1633b5, 0x3bb188, 0x1d3a9d, 0x4013d0, 0xcc1be5,
0xf862e7, 0x3bf28f, 0x39b5bf, 0x0bc235, 0x22747e, 0xa247c0,
0xd52d1f, 0x19add3, 0x9094df, 0x9311d0, 0xb42b25, 0x496db2,
0xe264b2, 0x5ef135, 0x3bc6a4, 0x1a4ad0, 0xaac92e, 0x64e886,
0x573091, 0x982cfb, 0x311b1a, 0x08728b, 0xbdcee1, 0x60e142,
0xeb641d, 0xd0bba3, 0xe559d4, 0x597b8c, 0x2a4483, 0xf332ba,
0xf84867, 0x2c8d1b, 0x2fa9b0, 0x50f3dd, 0xf9f573, 0xdb61b4,
0xfe233e, 0x6c41a6, 0xeea318, 0x775a26, 0xbc5e5c, 0xcea708,
0x94dc57, 0xe20196, 0xf1e839, 0xbe4851, 0x5d2d2f, 0x4e9555,
0xd96ec2, 0xe7d755, 0x6304e0, 0xc02e0e, 0xfc40a0, 0xbbf9b3,
0x7125a7, 0x222dfb, 0xf619d8, 0x838c1c, 0x6619e6, 0xb20d55,
0xbb5137, 0x79e809, 0xaf9149, 0x0d73de, 0x0b0da5, 0xce7f58,
0xac1934, 0x724667, 0x7a1a13, 0x9e26bc, 0x4555e7, 0x585cb5,
0x711d14, 0x486991, 0x480d60, 0x56adab, 0xd62f64, 0x96ee0c,
0x212ff3, 0x5d6d88, 0xa67684, 0x95651e, 0xab9e0a, 0x4ddefe,
0x571010, 0x836a39, 0xf8ea31, 0x9e381d, 0xeac8b1, 0xcac96b,
0x37f21e, 0xd505e9, 0x984743, 0x9fc56c, 0x0331b7, 0x3b8bf8,
0x86e56a, 0x8dc343, 0x6230e7, 0x93cfd5, 0x6a8f2d, 0x733005,
0x1af021, 0xa09fcb, 0x7415a1, 0xd56b23, 0x6ff725, 0x2f4bc7,
0xb8a591, 0x7fac59, 0x5c55de, 0x212c38, 0xb13296, 0x5cff50,
0x366262, 0xfa7b16, 0xf4d9a6, 0x2acfe7, 0xf07403, 0xd4d604,
0x6fd916, 0x31b1bf, 0xcbb450, 0x5bd7c8, 0x0ce194, 0x6bd643,
0x4fd91c, 0xdf4543, 0x5f3453, 0xe2b5aa, 0xc9aec8, 0x131485,
0xf9d2bf, 0xbadb9e, 0x76f5b9, 0xaf15cf, 0xca3182, 0x14b56d,
0xe9fe4d, 0x50fc35, 0xf5aed5, 0xa2d0c1, 0xc96057, 0x192eb6,
0xe91d92, 0x07d144, 0xaea3c6, 0x343566, 0x26d5b4, 0x3161e2,
0x37f1a2, 0x209eff, 0x958e23, 0x493798, 0x35f4a6, 0x4bdc02,
0xc2be13, 0xbe80a0, 0x0b72a3, 0x115c5f, 0x1e1bd1, 0x0db4d3,
0x869e85, 0x96976b, 0x2ac91f, 0x8a26c2, 0x3070f0, 0x041412,
0xfc9fa5, 0xf72a38, 0x9c6878, 0xe2aa76, 0x50cfe1, 0x559274,
0x934e38, 0x0a92f7, 0x5533f0, 0xa63db4, 0x399971, 0xe2b755,
0xa98a7c, 0x008f19, 0xac54d2, 0x2ea0b4, 0xf5f3e0, 0x60c849,
0xffd269, 0xae52ce, 0x7a5fdd, 0xe9ce06, 0xfb0ae8, 0xa50cce,
0xea9d3e, 0x3766dd, 0xb834f5, 0x0da090, 0x846f88, 0x4ae3d5,
0x099a03, 0x2eae2d, 0xfcb40a, 0xfb9b33, 0xe281dd, 0x1b16ba,
0xd8c0af, 0xd96b97, 0xb52dc9, 0x9c277f, 0x5951d5, 0x21ccd6,
0xb6496b, 0x584562, 0xb3baf2, 0xa1a5c4, 0x7ca2cf, 0xa9b93d,
0x7b7b89, 0x483d38,
};

int32_t
__ieee754_rem_pio2l (long double x, long double *y)
{
  double tx[3], ty[3];
  int32_t se, j0;
  uint32_t i0, i1;
  int sx;
  int n, exp;

  GET_LDOUBLE_WORDS (se, i0, i1, x);
  sx = (se >> 15) & 1;
  j0 = (se & 0x7fff) - 0x3fff;

  if (j0 < -1)
    {
      /* |x| < pi/4.  */
      y[0] = x;
      y[1] = 0;
      return 0;
    }

  if (j0 >= 0x8000)
    {
      /* x is infinite or NaN.  */
      y[0] = x - x;
      y[1] = y[0];
      return 0;
    }

  /* Split the 64 bits of the mantissa into three 24-bit integers
     stored in a double array.  */
  exp = j0 - 23;
  tx[0] = (double) (i0 >> 8);
  tx[1] = (double) (((i0 << 16) | (i1 >> 16)) & 0xffffff);
  tx[2] = (double) ((i1 << 8) & 0xffffff);

  n = __kernel_rem_pio2 (tx, ty, exp, 3, 2, two_over_pi);

  /* The result is now stored in two double values, we need to convert
     it into two long double values.  */
  if (sx == 0)
    {
      y[0] = (long double) ty[0] + (long double) ty[1];
      y[1] = ty[1] - (y[0] - ty[0]);
      return n;
    }
  else
    {
      y[0] = -((long double) ty[0] + (long double) ty[1]);
      y[1] = -ty[1] - (y[0] + ty[0]);
      return -n;
    }
}
