#!/usr/bin/env bash

#function that checks if id exists in array of ids
function exists() { #arg1:id, arg2: array(in or out)
    id=$1
    shift
    array=("$@")
    length=${#array[@]}
    counter=0
    while [[ $counter -lt $length ]]; do
        if [[ ${array[counter]} -eq $id ]]; then
            return 1;
        fi
        ((counter++))
    done
    return 0
}

iID=0 #: incoming ID
oID=0 #: otucoming ID
NBs=0 #: name bytes sent(file or directory name)
NBr=0 #: name bytes received
FBs=0 #: file bytes sent
FBr=0 #: file bytes received
LBs=0 #: length bytes sent (2 or 4)
LBr=0 #: length bytes received (2 or 4)
declare -a conIDs #list of different clients connected
declare -a dconIDs #list of different clients disconnected
Fs=0 #: number of files and directories sent
Fr=0 #: number of files and directories received

while read line;
do
    #echo $line #print line that was read
    IFS=' ' read -ra ADDR <<< "$line"
    type=${ADDR[0]}
    value=${ADDR[1]}

    if [[ $type == "iID" ]]; then
        if exists "${value}" "${conIDs[@]}"; then
            len=${#conIDs[@]} #geting length of array
            conIDs[len]=$value #writing in next position of last
        fi
        ((iID++))

    elif [[ $type == "oID" ]]; then
        if exists "$value" "${dconIDs[@]}"; then
            len=${#dconIDs[@]}
            dconIDs[len]=$value
        fi
        ((oID++))

    elif [[ $type == "NBs" ]]; then
        NBs=$((NBs + $value))
        LBs=$((LBs + 6)) #4 + 2
        #if size is 3, it's ..(previous folder)
        if [[ $type -ne 3 ]]; then
            ((Fs++))
        fi

    elif [[ $type == "NBr" ]]; then
        NBr=$((NBr + $value))
        LBr=$((LBr + 6))
        if [[ $value -ne 3 ]]; then
            ((Fr++))
        fi

    elif [[ $type == "FBs" ]]; then
        FBs=$((FBs + $value))

    elif [[ $type == "FBr" ]]; then
        FBr=$((FBr + $value))
    fi

done

#find maximum and minimum ID value(they should be in connected IDs)
MaxID=0 #: maximum client ID value
minID=${conIDs[0]} #: minimum client ID value
count=0
len=${#conIDs[@]}
while [[ $count -lt $len ]]; do
    #max
    if [[ ${conIDs[count]} -gt $MaxID ]]; then
        MaxID=${conIDs[count]}
    fi
    #min
    if [[ ${conIDs[count]} -lt $minID ]]; then
        minID=${conIDs[count]}
    fi
    ((count++))
done

echo "Total client connections: "$iID
echo "Total client disconnections: "$oID
echo "Total name bytes sent: "$NBs
echo "Total name bytes received: "$NBr
echo "Total file bytes sent: "$FBs
echo "Total file bytes received: "$FBr
echo "Total bytes sent for names and files lengths: "$LBs
echo "Total bytes received for names and files lengths: "$LBr
echo
echo "Total unique clients connected: "${#conIDs[@]}
echo ${conIDs[@]}
echo "Minimum id value: "$minID
echo "Maximum id value: "$MaxID
echo "Total bytes sent: "$((NBs + $FBs + $LBs))
echo "Total bytes received: "$((NBr + $FBr + $LBr))
echo "Total files and folders sent: "$Fs
echo "Total files and folders received: "$Fs
echo "Total unique clients disconnected: "${#dconIDs[@]}
echo ${dconIDs[@]}
