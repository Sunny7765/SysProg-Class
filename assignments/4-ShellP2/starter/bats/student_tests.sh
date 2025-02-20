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

@test "Error: Invalid command should fail" {
    run ./dsh <<EOF                
invalidcommand
EOF
    
    # Assertions
    [ "$status" -ne 0 ]
    [[ "$output" =~ "exec failure" ]]
}

@test "Built-in: Check exit command" {
    run ./dsh <<EOF                
exit
EOF
    
    # Assertions
    [ "$status" -eq 0 ]
}

