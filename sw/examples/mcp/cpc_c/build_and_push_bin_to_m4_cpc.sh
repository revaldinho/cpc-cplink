#!/bin/bash
#
# build_and_push_bin_to_m4_cpc.sh [-i|--ipaddr ip_addr] [-d|--destination destination_dir]
#

# Default ipaddr and target_dir to Shifters' original
ip_addr=192.168.0.55
target_dir=/

#ip_addr=192.168.1.114
#target_dir=/dev/mcp

POSITIONAL=()
while [[ $# -gt 0 ]]
do
    key="$1"

    case $key in
	-d|--destination)
	    target_dir="$2"
	    shift # past argument
	    shift # past value
	    ;;
	-i|--ipaddr)
	    ip_addr="$2"
	    shift # past argument
	    shift # past value
	    ;;
	--default)
	    DEFAULT=YES
	    shift # past argument
	    ;;
	*)    # unknown option
	    POSITIONAL+=("$1") # save it in an array for later
	    shift # past argument
	    ;;
    esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters


# Test IP addr
echo "Testing M4 IP address"
ping -c 1 $ip_addr
if [ $? -gt 0 ]
then
    echo "Error - could not contact M4 at IP addr $ip_addr"
    exit 1
fi

declare -a command_list=(
    "alloctest" 
    "date" 
    "helptest" 
    "ping" 
    "reboot" 
    "reset" 
    "shutdown" 
    "time" 
    "troff" 
    "tron" 
)


for command in "${command_list[@]}"
do
    echo "Building ${command}.bin"
    pushd $command
    make  
    echo "Sending ${command}.bin file to M4 on CPC"
    xfer -u ${ip_addr} obj/${command}.bin ${target_dir} 2 0x4000 0x4000
    rm -rf *cdt *dsk obj/
    popd
done
