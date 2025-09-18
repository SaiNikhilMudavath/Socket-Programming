#!/bin/bash

# Start the server in the background
./server_aloha &
SERVER_PID=$!

# Function to check if the server is running
is_server_running() {
    ps -p $SERVER_PID > /dev/null
}

until is_server_running; do
    sleep 1 # Check every second
done

./client_slotted_aloha

# Optionally, wait for the server to exit before finishing the script
wait $SERVER_PID
