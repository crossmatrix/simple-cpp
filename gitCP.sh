# git config --global core.editor '"C:\Users\Administrator\AppData\Local\Programs\Microsoft VS Code\bin\code"'  
## 为方便自动化执行，先使用上述命令将 --wait 参数去除

# output=$(git cherry-pick ec95b9d..1eb8c8d)
#output=$(git checkout . --theirs && git add . && git commit --allow-empty && git cherry-pick --continue)
output=$(git push)
echo "-----1"
echo $output

while :
do
	if [[ $output == "fatal"* ]]; then
		output=$(git push)
		echo "-----2"
		echo $output
	else
		echo "-----3"
		#break
	fi
done

echo "suc!"