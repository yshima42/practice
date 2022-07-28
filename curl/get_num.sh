i=0
while [ $i -lt 100 ]
do
	curl -X GET https://challenge-your-limits3.herokuapp.com/input/is/$i 2> /dev/null | jq '.output'
	i=$((i + 1))
done
