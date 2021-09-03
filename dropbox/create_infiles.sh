#!/usr/bin/env bash

dir_name=$1
num_of_files=$2
num_of_dirs=$3
levels=$4
re='^[0-9]+$' #numeric values


#####-1- checking input values #####

if ! [[ $num_of_files =~ $re ]]; then 
    echo "Wrong input number of files, setting it to twenty"
    num_of_files=20
fi
if ! [[ $num_of_dirs =~ $re ]]; then 
    echo "Wrong input number of directories, setting it to eight"
    num_of_dirs=8
fi
if ! [[ $levels =~ $re ]]; then 
    echo "Wrong input number of levels, setting it to two"
    levels=2
fi
# ((num_of_files += 500))
echo $num_of_files
# ((num_of_dirs += 600))
echo $num_of_dirs
# ((levels += 900))
echo $levels
echo


#####-2- checks if dir_name exists, if not, it creates it and use it #####

if ! [[ -d $dir_name ]]; then
    echo "Creating root directory with name $dir_name"
    mkdir $dir_name
fi
cd $dir_name


#####-3- making directory names array #####

#filling array of directory names
c=0
while [[ $c -lt $num_of_dirs ]]
do
    directory_names[c]=$( </dev/urandom tr -dc 'A-Za-z0-9' | head -c 8)
    ((c++))
done

#printing array of names
c=0
while [[ $c -lt $num_of_dirs ]]
do
    echo "Position "$c": "${directory_names[c]}
    ((c++))
done


#####-4- Making directories #####

c=0 #array counter for last file name
dir_counter=0 #directories counter
while [[ $dir_counter -lt $num_of_dirs ]];
do
    #making folder
    c=$dir_counter
    mkdir ${directory_names[dir_counter]}
    ((dir_counter++))
    
    #making subfolders(levels)    
    lvlcounter=1
    while [[ $lvlcounter -lt $levels ]] && [[ $dir_counter -lt $num_of_dirs ]];
    do
        cd ${directory_names[c]}
        c=$dir_counter
        mkdir ${directory_names[dir_counter]}
        ((dir_counter++))
        ((lvlcounter++))
    done

    while [[ $lvlcounter -gt 1 ]];
    do
        cd ..
        ((lvlcounter--))
    done

done

#####-5- Making files and filing folders with them #####

c=0 #array counter for last file name
dir_counter=0 #directories counter
file_counter=0 #file counter
while [[ $file_counter -lt $num_of_files ]];
do
    if [[ $dir_counter -eq $num_of_dirs ]]; then
        dir_counter=0
    fi
    
    #making file if directories are over and at start
    if [[ $dir_counter -eq 0 ]]; then
        filename=$( </dev/urandom tr -dc 'A-Za-z0-9' | head -c 8) #generating random name
        echo $filename" @@ "$PWD
        touch $filename #creating file
        #filling file
        rand=$((1000 + RANDOM % 127001))
        echo "SIZE: "$rand
        tr -dc 'A-Za-z0-9' </dev/urandom | head -c $rand > $filename
        ((file_counter++))
    fi
    
    #making files in subfolders(levels)    
    lvlcounter=0
    while [[ $lvlcounter -lt $levels ]] && [[ $dir_counter -lt $num_of_dirs ]] && [[ $file_counter -lt $num_of_files ]];
    do
        cd ${directory_names[dir_counter]}
        filename=$( </dev/urandom tr -dc 'A-Za-z0-9' | head -c 8) #generating random name
        echo $filename" at "$PWD
        touch $filename #creating file
        #filling file
        rand=$((1000 + RANDOM % 127001)) 
        echo "SIZE: "$rand
        tr -dc 'A-Za-z0-9' </dev/urandom | head -c $rand > $filename

        #steps
        ((file_counter++))
        ((dir_counter++))
        ((lvlcounter++))
    done

    while [[ $lvlcounter -gt 0 ]];
    do
        cd ..
        ((lvlcounter--))
    done
done