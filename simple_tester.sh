# #! /bin/bash
# if [ "$(uname)" = "Darwin" ]; then
# 	IRC_SERVER=$(ifconfig | grep "inet " | grep -v 127.0.0.1 | awk '{print $2}')
# else
# 	IRC_SERVER=$(hostname -I | awk '{print $1}')
# fi
# PORT=8000
# PASSWORD="p"
# USER1="user$1"
# USER2="full$1"
# NICK="nick$1"
# CHANNEL="#nc"
# {
# 	sleep 0.1
# # Connection requirements
# 	echo > /dev/tty;
# 	echo "PASS $PASSWORD";
# 	echo "NICK $NICK";
# 	echo "USER $USER1 * $IRC_SERVER :$USER2";
# 	sleep 0.1;
# # Hardcoded tests
# 	echo "PRIVMSG $NICK :Hello me"
# 	echo "JOIN $CHANNEL"
# 	# echo "INVITE rockbot $CHANNEL"
# 	# sleep 0.4
# 	# echo "PRIVMSG $CHANNEL :rock"
# 	# sleep 0.4
# 	# echo "KICK $CHANNEL rockbot :I'm crazy with power!"
# 	sleep 0.1
# 	echo "MODE $CHANNEL"
# 	echo "MODE $CHANNEL +l 5"
# 	# echo "MODE $CHANNEL +i"
# 	echo "MODE $CHANNEL +t"
# 	# echo "MODE $CHANNEL +k SabotagingPassword"
# 	echo "MODE $CHANNEL -o $NICK"
# 	echo "MODE $CHANNEL"
# 	sleep 0.1
# 	# echo "PART $CHANNEL"
# 	# sleep 0.1
# 	# echo "PRIVMSG rockbot ://rock"
# 	# sleep 0.1;
# 	# echo "PRIVMSG rockbot ://paper"
# 	# echo "PRIVMSG rockbot ://art"
# 	# echo "PRIVMSG rockbot ://rock"
# # Endof harcoded tests
# 	sleep 0.1;
# 	echo > /dev/tty;
# 	while [ true ]; do
# 		read -p "nc (or quit): " COMMAND;
# 		if [ "$COMMAND" = "quit" ]; then
# 			echo "QUIT :leaving...";
# 			break ;
# 		else
# 			echo "$COMMAND";
# 		fi
# 		sleep 0.1;
# 		echo > /dev/tty
# 	done
# } | tee /dev/tty | nc -C $IRC_SERVER $PORT
#
#
#
# #!/bin/bash
#
# # Check if the user provided a number of executions
# if [ -z "$1" ]; then
#     echo "Usage: $0 <number_of_executions>"
#     exit 1
# fi
#
# N=$1  # Number of times to run test.sh
#
# # Detect terminal emulator
# TERMINAL=""
# if command -v gnome-terminal >/dev/null; then
#     TERMINAL="gnome-terminal --"
# elif command -v konsole >/dev/null; then
#     TERMINAL="konsole -e"
# elif command -v xfce4-terminal >/dev/null; then
#     TERMINAL="xfce4-terminal -e"
# elif command -v xterm >/dev/null; then
#     TERMINAL="xterm -e"
# elif command -v lxterminal >/dev/null; then
#     TERMINAL="lxterminal -e"
# else
#     echo "No supported terminal found! Try installing gnome-terminal, konsole, or xterm."
#     exit 1
# fi
#
# # Loop to start test.sh instances in separate terminal windows
# for ((i = 1; i <= N; i++)); do
#
#     echo "Opening terminal for nctest.sh with arguments: $i"
#
#     # Open test.sh in a new terminal window
#     $TERMINAL bash -c "bash nctest.sh $i; exec bash" &
# done
#
# echo "All test.sh instances have been started in new windows."
