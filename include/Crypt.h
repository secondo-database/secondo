/*

Because this file is donwloaded from an extrenal source, no pd-comments are included.

*/


#ifndef CRYPT_H
#define CRYPT_H

 /*
  * This program implements the
  * Proposed Federal Information Processing
  *  Data Encryption Standard.
  * See Federal Register, March 17, 1975 (40FR12134)

  // converted to a c++ class by Thomas Behr
 */


class Crypt{

public: 

static char * crypt(const char* pw, const char* salt);
static bool validate(const char* pw, const char* epw);


private:

  

 /*
  * Initial permutation,
  */
static  char    IP[];
 /*
  * Final permutation, FP = IP^(-1)
  */
static  char    FP[];
 /*
  * Permuted-choice 1 from the key bits
  * to yield C and D.
  * Note that bits 8,16... are left out:
  * They are intended for a parity check.
  */
static  char    PC1_C[];
static  char    PC1_D[];
 /*
  * Sequence of shifts used for the key schedule.
 */
static  char    shifts[];
 /*
  * Permuted-choice 2, to pick out the bits from
  * the CD array that generate the key schedule.
  */
static  char    PC2_C[];

static  char    PC2_D[];
 /*
  * The C and D arrays used to calculate the key schedule.
  */

static  char    C[28];
static  char    D[28];
 /*
  * The key schedule.
  * Generated from the key.
  */
static  char    KS[16][48];

 /*
  * Set up the key schedule from the key.
  */

static void setkey(char* key);


 /*
  * The E bit-selection table.
  */
static  char    E[48];
static  char    e[];

 /*
  * The 8 selection functions.
  * For some reason, they give a 0-origin
  * index, unlike everything else.
  */
static  char    S[8][64];

 /*
 * P is a permutation on the selected combination
 * of the current L and key.
 */
static  char    P[];
 /*
 * The current block, divided into 2 halves.
 */
static  char    L[32], R[32];
static  char    tempL[32];
static  char    f[32];

 /*
 * The combination of the key and the input, before selection.
 */
static  char    preS[48];

 /*
 * The payoff: encrypt a block.
 */

static void encrypt(char* block, int edflag);




};

#endif

