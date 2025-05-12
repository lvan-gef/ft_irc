#!/bin/bash

# Usage: ./simple_tester.sh <port> <password>
# Simple tester that reconnects after every failed registration or command

if [ "$(uname)" = "Darwin" ]; then
    IRC_SERVER=$(ifconfig | grep "inet " | grep 127.0.0.1 | awk '{print $2}')
else
    IRC_SERVER=$(hostname -I | awk '{print $1}')
fi

PORT="$1"
PASSWORD="$2"
NICK="luuk"
USER1="hello"
CHANNEL="#nc"

register() {
    echo "PASS $PASSWORD"
    echo "NICK $NICK"
    echo "USER $NICK * $IRC_SERVER :$USER1"
    sleep 0.5
}

run_test() {
    {
        register
        for cmd in "$@"; do
            echo "$cmd"
            sleep 0.5
        done
        echo "QUIT :Leaving"
    } | tee /dev/tty | nc -C "$IRC_SERVER" "$PORT"
    sleep 1
}

run_raw_test() {
    {
        for cmd in "$@"; do
            echo "$cmd"
            sleep 0.5
        done
        echo "QUIT :Leaving"
    } | tee /dev/tty | nc -C "$IRC_SERVER" "$PORT"
    sleep 1
}

### Start tests

# Bad registration (invalid PASS), expect disconnect
run_raw_test \
    "PASS" \
    "NICK $NICK" \
    "USER $NICK * $IRC_SERVER :$USER1"

# Various malformed/invalid registration sequences (each reconnects)
run_raw_test "PASS $PASSWORD" "NICK" "USER $NICK * $IRC_SERVER :$USER1"
run_raw_test "PASS $PASSWORD" "NICK 1$NICK" "USER $NICK * $IRC_SERVER :$USER1"
run_raw_test "PASS $PASSWORD" "NICK $NICK 1" "USER $NICK * $IRC_SERVER :$USER1"
run_raw_test "PASS $PASSWORD" "NICK $NICK" "USER"
run_raw_test "PASS $PASSWORD" "NICK $NICK" "USER $NICK"
run_raw_test "PASS $PASSWORD" "NICK $NICK" "USER $NICK *"
run_raw_test "PASS $PASSWORD" "NICK $NICK" "USER $NICK * $IRC_SERVER"
run_raw_test "PASS $PASSWORD" "NICK $NICK" "USER $NICK * $IRC_SERVER :"
run_raw_test "PASS $PASSWORD" "NICK $NICK" "USER [$NICK * $IRC_SERVER :$USER1"

# Now do valid login followed by real commands
run_test
run_test "JOIN"
run_test "JOIN #"
run_test "JOIN #test"
run_test "TOPIC"
run_test "TOPIC $CHANNEL new topic"
run_test "TOPIC $CHANNEL nt"
run_test "TOPIC $CHANNEL"
run_test "MODE"
run_test "MODE +l -1"
run_test "PART"
run_test "KICK"
run_test "PRIVMSG"
run_test "INVITE"
run_test "USERHOST"
run_test "WHOIS"
