#/bin/sh

i=0
inp='TuTestConnectivityCodes.sec.tmpl'
outp="TuTestConnectivityCodes.sec"
bin_dir_path="${SECONDO_BUILD_DIR}/bin"
script_dir_path="${SECONDO_BUILD_DIR}/Algebras/OSM/Scripts"
inp_file_path="${script_dir_path}/${inp}"
outp_file_path="${script_dir_path}/${outp}"

export_dir_path="${script_dir_path}/Exported"
road1_dual='TRUE'
road2_dual='TRUE'
while (test ${i} -lt 65536)
do
   export_file_path="${export_dir_path}/${i}.txt"
   echo ${i}
   expr1="s#<Road1Dual>#${road1_dual}#g"
   expr2="s#<Road2Dual>#${road2_dual}#g"
   expr3="s#<ConnectivityCode>#${i}#g"
   expr4="s#<ExportFile>#${export_file_path}#g"
   sed -E ${expr1} "${inp_file_path}"| sed -E  ${expr2} | 
      sed -E  ${expr3} | sed -E  ${expr4} > ${outp_file_path} 
   ${bin_dir_path}/SecondoTTYNT -i ${outp_file_path}
   let i=$i+1
done

sh -c 'for i in `ls Exported/*.txt`; do echo "$i `tail -n 1 ${i}|cut -c 6-36`"|sed "s# #,#g"; done'>ConnectivityCodesVsTransitions.csv
