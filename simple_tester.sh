#! /bin/bash

# a simple tester to make sure we dont segfault any more of we dont have everything we expect...

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
{
	sleep 0.5

	echo > /dev/tty;
	echo "PASS";
	echo "NICK $NICK";
	echo "USER $NICK * $IRC_SERVER :$USER1";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK";
	echo "USER $NICK * $IRC_SERVER :$USER1";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK 1$NICK";
	echo "USER $NICK * $IRC_SERVER :$USER1";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK $NICK 1";
	echo "USER $NICK * $IRC_SERVER :$USER1";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK $NICK";
	echo "USER";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK $NICK";
	echo "USER $NICK";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK $NICK";
	echo "USER $NICK * ";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK $NICK";
	echo "USER $NICK * $IRC_SERVER";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK $NICK";
	echo "USER $NICK * $IRC_SERVER :";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK $NICK";
	echo "USER [$NICK * $IRC_SERVER :$USER1";
	sleep 0.5;

	echo "PASS $PASSWORD";
	echo "NICK $NICK";
	echo "USER $NICK * $IRC_SERVER :$USER1";
	sleep 0.5;

    echo "JOIN";
	sleep 0.5;

    echo "JOIN #";
	sleep 0.5;

    echo "JOIN #test";
	sleep 0.5;

    echo "TOPIC";
    sleep 0.5;

    echo "TOPIC $CHANNEL new topic";
    sleep 0.5;

    echo "TOPIC $CHANNEL nt";
    sleep 0.5;

    echo "TOPIC $CHANNEL";
    sleep 0.5;

    echo "MODE";
    sleep 0.5;

    echo "MODE +l -1";
    sleep 0.5;

    echo "PART";
    sleep 0.5;

    echo "KICK";
    sleep 0.5;

} | tee /dev/tty | nc -C $IRC_SERVER $PORT
