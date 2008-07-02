
/*
3.7 ~CSubMove~

*/
#include "PeriodicTypes.h"


namespace periodic{


void CSubMove::Equalize(const CSubMove* SM){
  SubMove::Equalize(SM);
  duration.Equalize(&(SM->duration));
}

} // end of namespace periodic

