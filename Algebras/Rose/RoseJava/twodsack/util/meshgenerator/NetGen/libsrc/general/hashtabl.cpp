/**************************************************************************/
/* File:   hashtabl.cpp                                                   */
/* Author: Joachim Schoeberl                                              */
/* Date:   01. Jun. 95                                                    */
/**************************************************************************/

/* 
   Abstract data type HASHTABLE
*/

#include <algorithm>
#include <mystdlib.h>
#include <myadt.hpp>

namespace netgen
{
  using namespace netgen;

  void INDEX_4 :: Sort ()
  {
    if (i[0] > i[1]) Swap (i[0], i[1]);
    if (i[2] > i[3]) Swap (i[2], i[3]);
    if (i[0] > i[2]) Swap (i[0], i[2]);
    if (i[1] > i[3]) Swap (i[1], i[3]);
    if (i[1] > i[2]) Swap (i[1], i[2]);
  }



  void INDEX_4Q :: Sort ()
  {
    if (min2 (i[1], i[2]) < min2 (i[0], i[3]))
      { Swap (i[0], i[1]); Swap (i[2], i[3]);}
    if (i[3] < i[0])
      { Swap (i[0], i[3]); Swap (i[1], i[2]);}
    if (i[3] < i[1])
      { Swap (i[1], i[3]); }
  }


  std::ostream & operator<<(std::ostream  & s, const INDEX_2 & i2)
  {
    return s << i2.I1() << ", " << i2.I2();
  }

  std::ostream & operator<<(std::ostream  & s, const INDEX_3 & i3)
  {
    return s << i3.I1() << ", " << i3.I2() << ", " << i3.I3();
  }

  std::ostream & operator<<(std::ostream  & s, const INDEX_4 & i4)
  {
    return s << i4.I1() << ", " << i4.I2() << ", " << i4.I3() 
             << ", " << i4.I4();
  }

  std::ostream & operator<<(std::ostream  & s, const INDEX_4Q & i4)
  {
    return s << i4.I1() << ", " << i4.I2() << ", " << i4.I3() 
             << ", " << i4.I4();
  }


  int BASE_INDEX_HASHTABLE :: Position (int bnr, const INDEX & ind) const
  {
    int i;
    for (i = 1; i <= hash.EntrySize (bnr); i++)
      if (hash.Get(bnr, i) == ind)
    return i;
    return 0;
  }



  /*
  int BASE_INDEX_2_HASHTABLE :: Position (int bnr, const INDEX_2 & ind) const
  {
    int i;
    for (i = 1; i <= hash.EntrySize (bnr); i++)
      if (hash.Get(bnr, i) == ind)
    return i;
    return 0;
  }
  */  

  void BASE_INDEX_2_HASHTABLE :: PrintStat (std::ostream & ost) const
  {
    int n = hash.Size();
    int i;
    int sumn = 0, sumnn = 0;

    for (i = 1; i <= n; i++)
      {
    sumn += hash.EntrySize(i);
    sumnn += sqr (hash.EntrySize(i));
      }

    ost << "Hashtable: " << std::endl
    << "size             : " << n << std::endl
    << "elements per row : " << (double(sumn) / double(n)) << std::endl
    << "av. acces time   : " 
    << (sumn ? (double (sumnn) / double(sumn)) : 0) << std::endl;
  }


  /*
    int BASE_INDEX_3_HASHTABLE :: Position (int bnr, const INDEX_3 & ind) const
    {
    int i;
    const INDEX_3 * pi = &hash.Get(bnr, 1);
    int n = hash.EntrySize(bnr);
    for (i = 1; i <= n; ++i, ++pi)
    {
    if (*pi == ind)
    return i;
    }

    return 0;
    }
  */

















  BASE_INDEX_2_CLOSED_HASHTABLE ::
  BASE_INDEX_2_CLOSED_HASHTABLE (int size)
    : hash(size)
  {
    hash.SetName ("i2-hashtable, hash");

    invalid = -1;
    int i;
    for (i = 1; i <= size; i++)
      hash.Elem(i).I1() = invalid;
  }

  void BASE_INDEX_2_CLOSED_HASHTABLE ::
  BaseSetSize (int size)
  {
    int i;
    hash.SetSize(size);
    for (i = 1; i <= size; i++)
      hash.Elem(i).I1() = invalid;
  }


  int BASE_INDEX_2_CLOSED_HASHTABLE ::
  Position2 (const INDEX_2 & ind) const
  {
    int i;

    i = HashValue(ind);
    while (1)
      {
    i++;
    if (i > hash.Size()) i = 1;
    if (hash.Get(i) == ind) return i;
    if (hash.Get(i).I1() == invalid) return 0;
      }
  }



  int BASE_INDEX_2_CLOSED_HASHTABLE ::
  PositionCreate2 (const INDEX_2 & ind, int & apos) 
  {
    int i;

    i = HashValue(ind);
    while (1)
      {
    i++;
    if (i > hash.Size()) i = 1;
    if (hash.Get(i) == ind) 
      {
        apos = i;
        return 0;
      }
    if (hash.Get(i).I1() == invalid) 
      {
        hash.Elem(i) = ind;
        apos = i;
        return 1;
      }
      }
  }

  int BASE_INDEX_2_CLOSED_HASHTABLE :: UsedElements () const
  {
    int i, n = hash.Size();
    int cnt = 0;
    for (i = 1; i <= n; i++)
      if (hash.Get(i).I1() != invalid)
    cnt++;
    return cnt;
  }











  BASE_INDEX_3_CLOSED_HASHTABLE ::
  BASE_INDEX_3_CLOSED_HASHTABLE (int size)
    : hash(size)
  {
    hash.SetName ("i3-hashtable, hash");

    invalid = -1;
    int i;
    for (i = 1; i <= size; i++)
      hash.Elem(i).I1() = invalid;
  }

  void BASE_INDEX_3_CLOSED_HASHTABLE ::
  BaseSetSize (int size)
  {
    int i;
    hash.SetSize(size);
    for (i = 1; i <= size; i++)
      hash.Elem(i).I1() = invalid;
  }


  int BASE_INDEX_3_CLOSED_HASHTABLE ::
  Position2 (const INDEX_3 & ind) const
  {
    int i;

    i = HashValue(ind);
    while (1)
      {
    i++;
    if (i > hash.Size()) i = 1;
    if (hash.Get(i) == ind) return i;
    if (hash.Get(i).I1() == invalid) return 0;
      }
  }



  int BASE_INDEX_3_CLOSED_HASHTABLE ::
  PositionCreate2 (const INDEX_3 & ind, int & apos) 
  {
    int i;

    i = HashValue(ind);
    while (1)
      {
    i++;
    if (i > hash.Size()) i = 1;
    if (hash.Get(i) == ind) 
      {
        apos = i;
        return 0;
      }
    if (hash.Get(i).I1() == invalid) 
      {
        hash.Elem(i) = ind;
        apos = i;
        return 1;
      }
      }
  }

  int BASE_INDEX_3_CLOSED_HASHTABLE :: UsedElements () const
  {
    int i, n = hash.Size();
    int cnt = 0;
    for (i = 1; i <= n; i++)
      if (hash.Get(i).I1() != invalid)
    cnt++;
    return cnt;
  }


}

