#!/bin/bash

# Reformat and compress html and other data

# Python3
py="python"

# Java
java="java"

# Closure compiler path
cc="../../utilities/ClosureCompiler/bin/closure-compiler-v20200406.jar"

# Compress tool
gztool="../../utilities/CompressToGZip/CompressToGZip.py"

# Temp dir
tempdir="temp/"

# Data dir
datadir="data/"

# Checking directory exists
check_dir(){
	file=$1
	prefix=$2
	if [ "x$file" == "x" ] || [ "x$prefix" == "x" ]; then
		echo "Not specified any file or prefix"
		return
	fi
	mkdir -p $(dirname "$prefix$file")
}

# Handle HTML
handle_html(){
	file=$1
	if [ "x$file" == "x" ] || [ ! -f $file ]; then
		echo "Not specified any file or there's no such file"
		return
	fi
	sed 's/^\s\+//g;s/\s\+$//g' $file | tr -d '\r\n' > $tempdir$file
	echo "-->" $tempdir$file
}

# Handle CSS
handle_css(){
	file=$1
	if [ "x$file" == "x" ] || [ ! -f $file ]; then
		echo "Not specified any file or there's no such file"
		return
	fi
	#test_min=$(basename -s .min.css "$file")
	test_min=$(basename "$file" .min.css)
	test_nor=$(basename "$file" .css)
	if [ "$test_min".min != "$test_nor" ]; then
		sed 's/^\s\+//g;s/\s\+$//g' $file | tr -d '\r\n' > $tempdir$file
	else
		cp $file $tempdir$file
	fi
	echo "-->" $tempdir$file
}

# Handle JS
handle_js(){
	file=$1
	if [ "x$file" == "x" ] || [ ! -f $file ]; then
		echo "Not specified any file or there's no such file"
		return
	fi
	#test_min=$(basename -s .min.js "$file")
	test_min=$(basename "$file" .min.js)
	test_nor=$(basename "$file" .js)
	if [ "$test_min".min != "$test_nor" ]; then
		# not a min file, do compress
		echo "========> Doing java compile... $file"
		$java -jar $cc --charset "UTF-8" --env BROWSER --js $file --js_output_file $tempdir$file
	else
		cp $file $tempdir$file
	fi
	echo "-->" $tempdir$file
}

# Handle others
handle_other(){
	file=$1
	if [ "x$file" == "x" ] || [ ! -f $file ]; then
		echo "Not specified any file or there's no such file"
		return
	fi
	cp $file $tempdir$file
	echo $tempdir$file
}

# Do compress
do_compress(){
	file=$1
	$py $gztool -i "$tempdir$file" -o "$datadir$file.gz" -m compress
}

# Check environments

if [ ! -x $py ]; then
	which $py > /dev/null
	if [ $? -ne 0 ]; then
		echo "There is no python3 in your environment"
		exit 1
	fi
fi

if [ ! -f $gztool ]; then
	echo "Cannot figure compress tool's location"
	exit 1
fi

if [ ! -f $cc ]; then
	echo "Cannot figure closure-compiler tool's location"
	exit 1
fi

if [ ! -d "$datadir" ]; then
	mkdir -p "$datadir"
fi

if [ ! -d "$tempdir" ]; then
	mkdir -p "$tempdir"
fi

file_list=$1
if [ "x$file_list" == "x" ] || [ ! -f "$file_list" ]; then
	echo "Please specific file list will be packaged"
	exit 1
fi

while read line; do
	if [ "x$line" == "x" ]; then
		continue
	fi
	suffix=$(echo ${line##*.})
	#suffix=$(echo ${suffix^^})
	suffix=$(echo $suffix | tr '[[:lower:]]' '[[:upper:]]')
	check_dir $line $tempdir
	check_dir $line $datadir
	case $suffix in
		"HTML" | "HTM")
			handle_html $line
		;;
		"CSS")
			handle_css $line
		;;
		"JS")
			handle_js $line
		;;
		*)
			handle_other $line
		;;
	esac
	echo "Handled file '$line' to '$tempdir$line'"
	if [ ! -f $tempdir$line ]; then
		echo "The previous step maybe fall, please check the command return normally."
		continue
	fi
	do_compress $line
	echo "Compressed file '$line' to '$datadir$line'"
done < $file_list

echo "Finished."
