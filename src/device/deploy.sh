#!/bin/bash
# Usage example
# ./deploy.sh 192.168.1.31 hoge
ip=$1
password=$2
echo "Copying core files"
for f in *.py; do
    cmd="python ../webrepl/webrepl_cli.py -p $password $f $ip:/"
    echo $cmd
    $cmd
done
echo "Copying app files"
for f in app/*.py; do
    cmd="python ../webrepl/webrepl_cli.py -p $password $f $ip:/app/"
    echo $cmd
    # $cmd
done