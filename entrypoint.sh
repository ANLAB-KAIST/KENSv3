#!/usr/bin/env bash

if ! [ "$(ls -A /workspace/)" ]; then
    echo "Initializing workspace..."
    cp -r /init/* /workspace/
fi

function grade() {
    if [ "$#" -lt 2 ]; then
        echo "Usage: grade [app_name] [parts ...]"
        exit 1
    fi
    echo "Grading $1: ${*:2}"
    tmp_src=$(mktemp -d)

    cp -r /init/* "$tmp_src"
    cp /submission/*Assignment.hpp "$tmp_src/app/$1/"
    cp /submission/*Assignment.cpp "$tmp_src/app/$1/"

    tmp_build=$(mktemp -d)
    cd "$tmp_build" || exit 1
    cmake -G Ninja "$tmp_src"
    cmake --build .

    RANDOM_SEED=${RANDOM_SEED:-0}
    TIMEOUT=${TIMEOUT:-300}
    for seed in ${RANDOM_SEED//,/ }; do
        for part in "${@:2}"; do
            for test in $("./app/$1/$1-$part" --gtest_list_tests | grep '^  *'); do
                timeout ${TIMEOUT} GTEST_OUTPUT="xml:/xml/${1}_${part}_${seed}_${test}.xml" RANDOM_SEED=$seed "./app/$1/$1-$part" --gtest_filter="*$test"
            done
        done
    done
}

case "$1" in
"dev") bash ;;
"grade") grade ${*:2} ;;
*) echo "please specify dev/grade" ;;
esac
