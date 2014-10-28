
set FEF="-Dfile.encoding=UTF-8"

set CP=.;lib\java_cup_v10k_runtime.jar;lib\jl0.4.jar;lib\14_os_jpedal.jar;lib\je.jar;lib\grappa1_2.jar;lib\batik.jar;secondoInterface\SecondoInterface.jar;%JAVA3DPATH%


LIBPATH=lib;%JAVA3DLIB%


set  HEAP=-Xmx1024M


java  %HEAP% -Djava.library.path=%LIBPATH%  %FEF% -classpath %CP% gui.MainWindow


