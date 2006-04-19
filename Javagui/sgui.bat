
SET CP="lib/java_cup_v10k_runtime.jar";"lib/jl0.4.jar";".";"lib/14_os_jpedal.jar";"lib/je.jar"

java  -Xmx580M -Djava.library.path=lib -classpath %CP% gui.MainWindow
