- Enable Algebra

    After that, insert a new entry in the file Algebras/Management/AlgebraList.i.cfg

    activate the algebra by modifying the file makefile.algebras in Secondo’s main directory.
    Insert the two lines:
    ALGEBRA_DIRS += Kafka
    ALGEBRAS     += KafkaAlgebra
    If the algebra uses third party libraries, add the line:
    ALGEBRA_DEPS += <libname>


- How to use makefiles in CLion

    https://www.jetbrains.com/help/clion/managing-makefile-projects.html
    https://blog.jetbrains.com/clion/2018/08/working-with-makefiles-in-clion-using-compilation-db/

    pip install compiledb
     add export PATH="$HOME/.local/bin:$PATH" to .bashrc, to make it work as described in
     https://github.com/nickdiego/compiledb/issues/37

    Usage:
     compiledb make

- Working with db

     Start SecondoTTYBDB
        cd $SECONDO_BUILD_DIR/bin
        SecondoTTYBDB

     restore database berlintest from berlintest;
     open database berlintest;

     query Staedte;


     query [const scircle value (9.0 10.0 20.0)];
     query perimeter([const scircle value (1 2 3)]);

    query "qwe" feed startsWithS["q"] consume;