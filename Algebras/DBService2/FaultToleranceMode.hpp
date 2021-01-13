#ifndef DBS_FAULT_TOLERANT_MODE_H
#define DBS_FAULT_TOLERANT_MODE_H


namespace DBService {
  /*
  1.1.1.1 \textit{faultToleranceMode}

  This member stores the target number of replicas that is read from the
  configuration file.

  */

  enum FaultToleranceMode
  {
      NONE = 0,
      DISK = 1,
      NODE = 2,
  };

}

#endif