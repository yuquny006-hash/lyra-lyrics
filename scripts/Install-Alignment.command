#!/bin/bash
cd "$(dirname "$0")"
bash ./setup-alignment-macos.sh
result=$?
echo "Press Return to close / 按回车关闭"
read -r _
exit "$result"
