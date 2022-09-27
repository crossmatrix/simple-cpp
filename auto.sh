while :
do
	if git push; then
		break
	else
		echo "try again"
	fi
done

echo "suc!"