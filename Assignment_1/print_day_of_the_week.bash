month=(31 28 31 30 31 30 31 31 30 31 30 31)
day=(목요일 금요일 토요일 일요일 월요일 화요일 수요일)

echo -n "몇 월 며칠 입니까? 두 정수로 입력하세요 : "
read a b

i=0
while (( i < ($a - 1) ))
do
	b=`expr $b + ${month[i]}`
	let i++
done

index=`expr $b % 7`
echo ${day[index]}
