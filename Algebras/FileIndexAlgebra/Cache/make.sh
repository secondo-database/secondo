# Kompiliert die Dateien, die zum Cache gehoeren und
# das Beispielprogramm (Sample.cpp)

# g++
# -S         Compile only; do not assemble or link
# -c         Compile and assemble, but do not link
# -o <file>  Place the output into <file>


# Caches kompilieren
g++ -c DummyNode.h
g++ -c CacheBase.cpp
g++ -c NoCache.cpp
g++ -c IncLruCache.h -std=c++11
g++ -c LruCache.cpp -std=c++11


# Beispielprogramm kompilieren und linken
g++ -c Sample.cpp -std=c++11
g++ -o Sample -std=c++11 Sample.cpp CacheBase.o NoCache.o DummyNode.h IncLruCache.h LruCache.o

# Ausfuehren
#Sample