#!/usr/bin/env bash

declare -A submission_files=(
    ["TCPAssignment.cpp"]="app/kens/TCPAssignment.cpp"
    ["TCPAssignment.hpp"]="app/kens/TCPAssignment.hpp"
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

    tmp_build=$(mktemp -d)
    cd "$tmp_build" || exit 1
    cmake -G Ninja "$tmp_src"
    cmake --build .

    for part in "$@"; do
        "./app/kens/kens-part$part"
    done
}

case "$1" in
"dev") bash ;;
"grade") grade ${*:2} ;;
*) echo "please specify dev/grade" ;;
esac
