# ~/.bash_profile: executed by bash(1) for login shells.
# see /usr/share/doc/bash/examples/startup-files for examples.
# the files are located in the bash-doc package.

# the default umask is set in /etc/login.defs
#umask 022
set -o vi
#export CVSROOT=:pserver:cvs/home/cvs/cvsroot
export CVSROOT=:pserver:kevinr@cvs/home/cvs/cvsroot
#keep slackware hosts happy
export TERM=xterm

# include .bashrc if it exists
if [ -f ~/.bashrc ]; then
    . ~/.bashrc
fi

# set PATH so it includes user's private bin if it exists
if [ -d ~/bin ] ; then
    PATH=~/bin:"${PATH}"
fi
PATH=$PATH:/usr/sbin:/sbin