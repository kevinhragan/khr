# ~/.bash_profile: executed by bash(1) for login shells.
umask 002
set -o vi
export PS1='\w\$ '
export PATH=$PATH:~/bin:/usr/local/j2re1.4.2/bin:/sbin:/usr/sbin
export VISUAL=/usr/bin/vi
alias con='sudo ifdown wlan0; sudo ifup wlan0'
. ~/.bashrc
startx
