#/bin/sh

SecondoTTYNT <<-EOF 
list algebras;
quit;
EOF 
> alglist.out 

cat alglist.out | grep -i imexalgebra

