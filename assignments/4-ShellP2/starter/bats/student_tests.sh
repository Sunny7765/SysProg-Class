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

@test "Built-in: Change directory to /tmp and check pwd" {
    run ./dsh <<EOF                
cd /tmp
pwd
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
    [[ "$output" =~ "/tmp" ]]
}

@test "Built-in: Change directory to home with no args" {
    current=$(pwd)
    run ./dsh <<EOF                
cd
pwd
EOF
    
    expected_output=$(eval echo ~$USER)
    [ "$status" -eq 0 ]
    [[ "$output" =~ "$expected_output" ]]
}

@test "Built-in: Check exit command" {
    run ./dsh <<EOF                
exit
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
}

@test "Built-in: Change directory to /home" {
    run ./dsh <<EOF                
cd /home
pwd
EOF

    # Assertions
    [ "$status" -eq 0 ]  # Ensure the command was successful
    [[ "$output" =~ "/home" ]]  # Ensure the output shows the correct directory
}

@test "Echo command with special characters" {
    run "./dsh" <<EOF
echo "Hello\\ World"
EOF

    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')

    expected_output="Hello\ Worlddsh2> dsh2> cmd loop returned 0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    [ "$stripped_output" = "$expected_output" ]

    [ "$status" -eq 0 ]
}

@test "Display file content using cat" {
    echo "Hello this is a test" > test_cat.txt

    run "./dsh" <<EOF                
cat test_cat.txt
EOF

    expected_output="Hellothisisatestdsh2>dsh2>cmdloopreturned0"

    stripped_output=$(echo "$output" | tr -d '[:space:]')

    echo "Captured stdout:"
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    rm -f test_cat.txt

    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}