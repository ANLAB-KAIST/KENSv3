#!/usr/bin/env bash

declare -A submission_files=(
    ["TCPAssignment.cpp"]="app/TestTCP/TCPAssignment.cpp"
    ["TCPAssignment.hpp"]="app/TestTCP/TCPAssignment.hpp"
)

declare -A kens_tests=(
    ["1"]="TestEnv_Reliable.TestOpen:TestEnv_Reliable.TestBind_*"
    ["2"]="TestEnv_Reliable.TestAccept_*:TestEnv_Any.TestAccept_*:TestEnv_Any.TestConnect_*:TestEnv_Any.TestClose_*"
    ["3"]="TestEnv_Any.TestTransfer_*"
    ["4"]="TestEnv_Congestion*"
)

if ! [ "$(ls -A /workspace/)" ]; then
    echo "Initializing workspace..."
    cp -r /init/* /workspace/
fi

function grade() {
    echo "Grading part $*"
    tmp_src=$(mktemp -d)

    cp -r /init/* "$tmp_src"

    for f in "${!submission_files[@]}"; do
        cp "/submission/$f" "$tmp_src/${submission_files[$f]}"
    done

    cd "$tmp_src" || exit 1
    make -j

    for part in "$@"; do
        ./build/testTCP --gtest_filter=${kens_tests[$part]}
    done
}

case "$1" in
"dev") bash ;;
"grade") grade ${*:2} ;;
*) echo "please specify dev/grade" ;;
esac
