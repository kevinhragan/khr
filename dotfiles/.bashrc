# ~/.bashrc: executed by bash(1) for non-login shells.
set -o vi
set show-all-if-ambiguous on
alias rxvt="rxvt-xterm -title XTERM -font 6x13 -sb -ls&"
#alias ll="ls -l"
function ll { ls -l *;}
alias nap="sudo apm -S"
alias rewi="sudo ifdown wlan0; sudo ifup  wlan0"
if [ -e /etc/bash_completion ]; then
. /etc/bash_completion 
fi
