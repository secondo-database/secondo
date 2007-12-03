/*
\newpage

3.5.4 Implementation part (file: MTreeConfig.cpp)

*/
#include "MTreeAlgebra.h"
#include "MTreeConfig.h"

/*
Initialise static members :

*/
bool MT::MTreeConfigReg::initialized = false;
map< string, MT::MTreeConfig > MT::MTreeConfigReg::mTreeConfig_map;

/*
Method ~registerMTreeConfig~ :

*/
void
MT::MTreeConfigReg::registerMTreeConfig( const string& name,
                                     const MTreeConfig& config )
{
  mTreeConfig_map[ name ] = config;
}

/*
Method ~getMTreeConfig~ :

*/
MT::MTreeConfig
MT::MTreeConfigReg::getMTreeConfig( const string& name )
{
  if (!initialized)
    initialize();

  map< string, MTreeConfig >::iterator pos =
      mTreeConfig_map.find( name );

  if ( pos != mTreeConfig_map.end() )
  {
    #ifdef __MT_PRINT_CONFIG_INFO
    string promFunStr, partFunStr;
    switch ( pos->second.promoteFun )
    {
      case RANDOM:
        promFunStr = "random";
        break;
      case m_RAD:
        promFunStr = "minmal sum of covering radii";
        break;
      case mM_RAD:
        promFunStr = "minimal maximum of covering radii";
        break;
      case M_LB_DIST:
        promFunStr = "maximum lower bound on distance";
        break;
    }
    switch ( pos->second.partitionFun )
    {
      case GENERALIZED_HYPERPLANE:
        partFunStr = "generalized hyperplane";
        break;
      case BALANCED:
        partFunStr = "balanced";
        break;
    }
    cmsg.info() << endl
                << "Found mtree-config: " << endl
                << "----------------------------------------" << endl
                << "max entries per node: "
                << pos->second.maxNodeEntries << endl
                << "promote function: " << promFunStr << endl
                << "partition function: " << partFunStr << endl
                << endl;
    cmsg.send();
    #endif
    return pos->second;
  }
  else
  {
    #ifdef MT_PRINT_CONFIG_INFO
    cmsg.info() << "No mtree-config found, using default values."
                << endl;
    cmsg.send();
    #endif
    return MTreeConfig();
  }
}

/*
Method ~initialize~ :

*/
void
MT::MTreeConfigReg::initialize()
{
  registerMTreeConfig( "default",  MTreeConfig() );

  registerMTreeConfig( "rand",
      MTreeConfig(
          80,      // maxNodeEntries
          RANDOM,  // promote function (min. covering radius)
          BALANCED // partition function
      ));

  registerMTreeConfig( "mRad",
      MTreeConfig(
          80,      // maxNodeEntries
          m_RAD,   // promote function (min. covering radius)
          BALANCED // partition function
      ));

  registerMTreeConfig( "mMRad",
      MTreeConfig(
          80,      // maxNodeEntries
          mM_RAD,   // promote function (min. covering radius)
          BALANCED // partition function
      ));

  registerMTreeConfig( "mMRadHP",
      MTreeConfig(
          80,      // maxNodeEntries
          mM_RAD,   // promote function (min. covering radius)
          GENERALIZED_HYPERPLANE // partition function
      ));
  initialized = true;
}
