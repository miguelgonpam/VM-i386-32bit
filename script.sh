f="syscallent.h"
n=`cat $f | wc -l`

res="const int args[] = {"

for (( i=1; i<=$n; i++ ))
do
    l=`awk NR==$i $f`
    #echo $l
    nn=`echo $l | cut -f 1 -d "," | cut -f 2 -d "{"`
    res+=$nn
    res+=","


done 

res+="};"

echo $res

