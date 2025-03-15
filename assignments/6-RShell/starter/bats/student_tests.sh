#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

# Helper function to start the server in the background
@test "Check ls runs without errors" {
    run ./dsh <<EOF
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Check if dsh starts successfully" {
    run ./dsh <<EOF
exit
EOF

    [ "$status" -eq 0 ]
}

@test "Check pwd command execution" {
    run ./dsh <<EOF
pwd
EOF

    [ "$status" -eq 0 ]
    [ -n "$output" ]
}

@test "Check exit command execution" {
    run ./dsh <<EOF
exit
EOF

    [ "$status" -eq 0 ]
    [ -n "$output" ]
}

@test "Check stop-server command execution" {
    run ./dsh <<EOF
stop-server
EOF

    [ "$status" -eq 0 ]
    [ -n "$output" ]
}

@test "Check valid 'cd' command" {
    run ./dsh <<EOF
cd bats
EOF
    [ "$status" -eq 0 ]
}

@test "Check echo command execution" {
    run ./dsh <<EOF
echo "Hello, World!"
EOF

    [ "$status" -eq 0 ]
    echo "$output" | grep -q "Hello, World!"
}

@test "Check piping between commands" {
    run ./dsh <<EOF
echo "Test pipe" | grep "pipe"
EOF

    [ "$status" -eq 0 ]
    echo "$output" | grep -q "Test pipe"
}

#test cases for client/server

@test "Check valid 'ls' command on client/server side" {
    ./dsh -s &

    server_pid=$!

    sleep 1

    run ./dsh -c <<EOF
ls
exit
EOF

    [ "$status" -eq 0 ]

    if ps -p $server_pid > /dev/null; then
        echo "Killing server process with PID $server_pid"
        kill $server_pid
    else
        echo "Server process already terminated."
    fi
}

@test "Check if dsh starts successfully on client/server side" {
    ./dsh -s &

    server_pid=$!

    sleep 1

    run ./dsh -c <<EOF
exit
EOF

    [ "$status" -eq 0 ]

    if ps -p $server_pid > /dev/null; then
        echo "Killing server process with PID $server_pid"
        kill $server_pid
    else
        echo "Server process already terminated."
    fi
}

@test "Check valid 'pwd' command on client/server side" {
    ./dsh -s &

    server_pid=$!

    sleep 1

    run ./dsh -c <<EOF
pwd
exit
EOF

    [ "$status" -eq 0 ]

    if ps -p $server_pid > /dev/null; then
        echo "Killing server process with PID $server_pid"
        kill $server_pid
    else
        echo "Server process already terminated."
    fi
}

@test "Check valid 'exit' command on client/server side" {
    ./dsh -s &

    server_pid=$!

    sleep 1

    run ./dsh -c <<EOF
exit
EOF

    [ "$status" -eq 0 ]

    if ps -p $server_pid > /dev/null; then
        echo "Killing server process with PID $server_pid"
        kill $server_pid
    else
        echo "Server process already terminated."
    fi
}

@test "Check valid 'stop-server' command on client/server side" {
    ./dsh -s &

    server_pid=$!

    sleep 1

    run ./dsh -c <<EOF
stop-server
EOF

    [ "$status" -eq 0 ]

    if ps -p $server_pid > /dev/null; then
        echo "Killing server process with PID $server_pid"
        kill $server_pid
    else
        echo "Server process already terminated."
    fi
}

@test "Check valid 'cd' command on client/server side" {
    ./dsh -s &

    server_pid=$!

    sleep 1

    run ./dsh -c <<EOF
cd bats
exit
EOF

    echo "Client output: $output"

    echo "$output" | grep -q "Successfully changed directory to bats"
    [ "$status" -eq 0 ]

    if ps -p $server_pid > /dev/null; then
        echo "Killing server process with PID $server_pid"
        kill $server_pid
    else
        echo "Server process already terminated."
    fi
}

@test "Check invalid 'cd' command on client/server side" {
    ./dsh -s &

    server_pid=$!

    sleep 1

    run ./dsh -c <<EOF
cd invalidDirectory
exit
EOF

    echo "Client output: $output"

    echo "$output" | grep -q "Successfully changed directory to invalidDirectory"
    [ "$status" -eq 0 ]

    if ps -p $server_pid > /dev/null; then
        echo "Killing server process with PID $server_pid"
        kill $server_pid
    else
        echo "Server process already terminated."
    fi
}

@test "Check valid 'echo' command on client/server side" {
    ./dsh -s &

    server_pid=$!

    sleep 1

    run ./dsh -c <<EOF
echo "Hello, World!"
exit
EOF

    [ "$status" -eq 0 ]

    if ps -p $server_pid > /dev/null; then
        echo "Killing server process with PID $server_pid"
        kill $server_pid
    else
        echo "Server process already terminated."
    fi
}

@test "Check valid piping between commands on client/server side" {
    ./dsh -s &

    server_pid=$!

    sleep 1

    run ./dsh -c <<EOF
ls | grep dshlib.c
exit
EOF

    [ "$status" -eq 0 ]

    if ps -p $server_pid > /dev/null; then
        echo "Killing server process with PID $server_pid"
        kill $server_pid
    else
        echo "Server process already terminated."
    fi
}