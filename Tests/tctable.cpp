#include <string>
#include "CTable.h"

using namespace std;

int main()
{
  CTable<string> ct( 5 );
  cout << ct.Size() << endl;
  ct[2] = "Anton";
  string& strRef = ct[2];
  cout << strRef << endl;
  strRef = "Antonia";
  ct[4] = "Berta";
  cout << ct[2] << endl;
  cout << ct.Size() << endl;
  cout << ct[4] << endl;
  cout << "1: " << ct.IsValid( 1 ) << endl;
  cout << "2: " << ct.IsValid( 2 ) << endl;
  cout << "3: " << ct.IsValid( 3 ) << endl;
  cout << "4: " << ct.IsValid( 4 ) << endl;
  cout << "5: " << ct.IsValid( 5 ) << endl;
  ct.Add( "Dora" );
  ct.Add( "Emil" );
  ct.Add( "Floriane" );
  ct.Add( "Gerda" );
  ct.Add( "Hugo" );
  cout << "Size=" << ct.Size() << endl;
  CTable<string>::Iterator it, it2;
  it2 = ct.Begin();
  it = it2++;
  cout << "it  " << *it  << ", i= " << it.GetIndex()  << endl;
  *it = "Kasimir";
  cout << "it2 " << *it2 << ", i2=" << it2.GetIndex() << endl;
  ct.Remove( 5 );
  for ( it = ct.Begin(); it != ct.End(); ++it )
  {
    cout << "it " << *it << ", i=" << it.GetIndex() << endl;
  }
  cout << "eos? " << it.EndOfScan() << endl;
  ++it;
  cout << "eos? " << it.EndOfScan() << endl;
  return 0;
}

