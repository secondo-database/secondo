
# core operators
operator maxcore alias MAXCORE pattern op()
operator setcore alias SETCORE pattern op(_)
operator getcore alias GETCORE pattern op()
operator mThreadedMergeSort alias MTHREADEDMERGESORT pattern _ op [list]
operator mThreadedHybridJoin alias MTHREADEDHYBRIDJOIN pattern _ _ op [_, _]
operator mThreadedSpatialJoin alias MTHREADEDSPATIALJOIN pattern _ _ op [ fun ] implicit parameters streamelem1, streamelem2 types STREAMELEM, STREAMELEM2 !!
