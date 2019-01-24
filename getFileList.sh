#https://stackoverflow.com/questions/3795512/delete-the-first-five-characters-on-any-line-of-a-text-file-in-linux-with-sed
#otherwise each filename would begin with ./
find . \( -name \*.h -o -name \*.cpp \) | sed 's/^.\{,2\}//' > filelist.txt
