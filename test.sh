for x in test/??
do
	./srank <$x >/tmp/.srank
	cmp -s ${x}o /tmp/.srank
	if test $? != 0
	then
		echo "$x: failed"
		cat /tmp/.srank
	fi
done
