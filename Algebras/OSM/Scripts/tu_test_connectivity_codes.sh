#/bin/sh

i=0
inp='TuTestConnectivityCodes.sec.tmpl'
outp="TuTestConnectivityCodes.sec"
bin_dir_path="${SECONDO_BUILD_DIR}/bin"
script_dir_path="${SECONDO_BUILD_DIR}/Algebras/OSM/Scripts"
while (test ${i} -lt 65536)
do
   echo ${i}
   expr1="s#<Road1Dual>#${road1_dual}#g"
   expr2="s#<Road2Dual>#${road2_dual}#g"
   expr3="s#<ConnectivityCode>#${i}#g"
   sed -E ${expr1} "${script_dir_path}/${inp}"| sed -E  ${expr2} | 
      sed -E  ${expr3} > "${script_dir_path}/${outp}"
   ${bin_dir_path}/SecondoTTYNT -i "${script_dir_path}/TuTestConnectivityCode.sec"
   let i=$i+1
done
