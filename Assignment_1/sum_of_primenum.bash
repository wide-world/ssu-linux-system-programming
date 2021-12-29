echo -n "두 개의 정수를 입력하시오 : "
read a b
if (( $a > $b ))
then
	temp=$a
	a=$b
	b=$temp
fi

let a++
while (( $a < $b ))
do
	i=2
	while (( $i <= $a ))
	do
		if (( ($a % i) == 0 ))
		then
			break
		fi
		let i++
	done
	
	if [ $a -eq $i ]
	then
		sum=`expr $sum + $a`
	fi
	let a++
done

echo $sum
