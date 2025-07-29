f="syscallent.h"
ff="arch.h"
n=`cat $f | wc -l`

for (( i=1; i<=$n; i++ ))
do
    l=`awk NR==$i $f`
    #echo $l
    nn=`echo $l | cut -f 2 -d "\""`
    echo "uint32_t $nn (uint32_t *nr, uint32_t *arg1, uint32_t *arg2, uint32_t *arg3, uint32_t *arg4, uint32_t *arg5, uint32_t *arg6);" >> $ff
    #res+=$nn
    #res+=","


done 



#echo $res

