#!/usr/bin/env bash

declare -A submission_files=(
    ["TCPAssignment.cpp"]="app/kens/TCPAssignment.cpp"
    ["TCPAssignment.hpp"]="app/kens/TCPAssignment.hpp"
    ["RoutingAssignment.cpp"]="app/routing/RoutingAssignment.cpp"
    ["RoutingAssignment.hpp"]="app/routing/RoutingAssignment.hpp"
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
        if [ -f "/submission/$f" ]; then
            cp "/submission/$f" "$tmp_src/${submission_files[$f]}"
        fi
    done

    tmp_build=$(mktemp -d)
    cd "$tmp_build" || exit 1
    cmake -G Ninja "$tmp_src"
    cmake --build .

    for part in "$@"; do
        "./app/$part"
    done
}

case "$1" in
"dev") bash ;;
"grade") grade ${*:2} ;;
*) echo "please specify dev/grade" ;;
esac
