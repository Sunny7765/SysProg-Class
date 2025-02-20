#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Pipes: ls | grep" {
    run "./dsh" <<EOF
ls | grep dshlib.c
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dshlib.cdsh3>dsh3>cmdloopreturned0"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Pipes: echo | cat" {
    run "./dsh" <<EOF
echo hello world | cat
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="helloworlddsh3>dsh3>cmdloopreturned0"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Pipes: ls -l | grep dshlib" {
    run "./dsh" <<EOF
ls -l | grep dshlib
EOF
    if ! grep -q 'dshlib.c' <<< "$output" || ! grep -q 'dshlib.h' <<< "$output"; then
        echo "Output does not contain expected 'dshlib.c' and/or 'dshlib.h':"
        echo "$output"
        false 
    fi
    [ "$status" -eq 0 ]
}

@test "Pipes: Multiple pipes - ls -l | grep dsh | wc -l" {
    run "./dsh" <<EOF
ls -l | grep dsh | wc -l
EOF
    if ! grep -Eq '^[0-9]+$' <<< "$output"; then
        echo "Output is not a single number as expected from wc -l:"
        echo "$output"
        false 
    fi
    [ "$status" -eq 0 ]
}

@test "Pipes: Commands with arguments - nosuchcommand | grep word" {
    run "./dsh" <<EOF
echo "This is a test string with the word word in it" | grep word
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="Thisisateststringwiththewordwordinitdsh3>dsh3>cmdloopreturned0"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "Pipes: Command not found in pipeline | cat" {
    run "./dsh" <<EOF
nosuchcommand | cat
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    expected_output="execvp:Nosuchfileordirectorydsh3>dsh3>dsh3>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]

    [ "$status" -eq 0 ]
}

@test "Pipes: Too many commands - Exceeding CMD_MAX (adjust number if CMD_MAX is different)" {
    run "./dsh" <<EOF
echo 1 | echo 2 | echo 3 | echo 4 | echo 5 | echo 6 | echo 7 | echo 8 | echo 9 | echo 10
EOF
    if ! grep -q 'piping limited to' <<< "$output"; then
        echo "Error message 'piping limited to' not found in output:"
        echo "$output"
        false 
    fi
    [ "$status" -eq 0 ]
}

@test "Pipes: Redundant pipes - ls || grep c" {
    run "./dsh" <<EOF
ls || grep c
EOF
    if ! grep -q 'dshlib.c' <<< "$output"; then
        echo "Output does not contain 'dshlib.c':"
        echo "$output"
        false
    fi
    [ "$status" -eq 0 ]
}