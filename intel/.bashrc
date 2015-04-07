# ~/.bashrc: executed by bash(1) for non-login shells.
# see /usr/share/doc/bash/examples/startup-files (in the package bash-doc)
# for examples

# If not running interactively, don't do anything
case $- in
    *i*) ;;
      *) return;;
esac

# don't put duplicate lines or lines starting with space in the history.
# See bash(1) for more options
HISTCONTROL=ignoreboth

# append to the history file, don't overwrite it
shopt -s histappend

# for setting history length see HISTSIZE and HISTFILESIZE in bash(1)
HISTSIZE=1000
HISTFILESIZE=2000

# check the window size after each command and, if necessary,
# update the values of LINES and COLUMNS.
shopt -s checkwinsize

# If set, the pattern "**" used in a pathname expansion context will
# match all files and zero or more directories and subdirectories.
#shopt -s globstar

# make less more friendly for non-text input files, see lesspipe(1)
#[ -x /usr/bin/lesspipe ] && eval "$(SHELL=/bin/sh lesspipe)"

# set variable identifying the chroot you work in (used in the prompt below)
if [ -z "${debian_chroot:-}" ] && [ -r /etc/debian_chroot ]; then
    debian_chroot=$(cat /etc/debian_chroot)
fi

#set a fancy prompt (non-color, unless we know we "want" color)
case "$TERM" in
    xterm-color) color_prompt=yes;;
esac

# uncomment for a colored prompt, if the terminal has the capability; turned
# off by default to not distract the user: the focus in a terminal window
# should be on the output of commands, not on the prompt
force_color_prompt=yes

if [ -n "$force_color_prompt" ]; then
    if [ -x /usr/bin/tput ] && tput setaf 1 >&/dev/null; then
	# We have color support; assume it's compliant with Ecma-48
	# (ISO/IEC-6429). (Lack of such support is extremely rare, and such
	# a case would tend to support setf rather than setaf.)
	color_prompt=yes
    else
	color_prompt=
    fi
fi

if [ "$color_prompt" = yes ]; then
    #PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
    #PS1='${debian_chroot:+($debian_chroot)}\[\033[01;30m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
    PS1='\[\033[0;37m\]\w\[\033[00m\]\$ '
else
    PS1='${debian_chroot:+($debian_chroot)}\u@\h:\w\$ '
fi
unset color_prompt force_color_prompt

# If this is an xterm set the title to user@host:dir
case "$TERM" in
xterm*|rxvt*)
    PS1="\[\e]0;${debian_chroot:+($debian_chroot)}\u@\h: \w\a\]$PS1"
    ;;
*)
    ;;
esac

# enable color support of ls and also add handy aliases
if [ -x /usr/bin/dircolors ]; then
    test -r ~/.dircolors && eval "$(dircolors -b ~/.dircolors)" || eval "$(dircolors -b)"
    alias ls='ls --color=auto'
    #alias dir='dir --color=auto'
    #alias vdir='vdir --color=auto'

    #alias grep='grep --color=auto'
    #alias fgrep='fgrep --color=auto'
    #alias egrep='egrep --color=auto'
fi

# some more ls aliases
#alias ll='ls -l'
#alias la='ls -A'
#alias l='ls -CF'

# Alias definitions.
# You may want to put all your additions into a separate file like
# ~/.bash_aliases, instead of adding them here directly.
# See /usr/share/doc/bash-doc/examples in the bash-doc package.

if [ -f ~/.bash_aliases ]; then
    . ~/.bash_aliases
fi

# enable programmable completion features (you don't need to enable
# this, if it's already enabled in /etc/bash.bashrc and /etc/profile
# sources /etc/bash.bashrc).
if ! shopt -oq posix; then
  if [ -f /usr/share/bash-completion/bash_completion ]; then
    . /usr/share/bash-completion/bash_completion
  elif [ -f /etc/bash_completion ]; then
    . /etc/bash_completion
  fi
fi

############localizations#####################
alias rd="rdesktop -u Administrator -p Vmware00 -a 16 -g 1200x950 VM
"
# export TEST_SCRIPTS=/root/testing ; #. /root/testing/config/init_test.sh
alias tmsg='tail -f -n 100 /var/log/messages'
alias gitl="git log --color --pretty=format:'%h %an %s'"
alias xl="xscreensaver-command -lock"

neighbor() {
    lldptool -tni $1|egrep 'IPv4:|Local:|Ifname:'
}


rx() {
echo -ne "\033]0;$1\007"
ssh $1
}

# setup resources for all urxvts and remove this alias
localt() { urxvt -bg grey50 +sb; }
   

alias 192='rx 192'
alias 194='rx 194'
alias 195='rx 195'
alias 165='rx 165'
alias 166='rx 166'
alias 167='rx 167'
alias 168='rx 168'


fcoe2=eth2.172-fcoe
fcoe3=eth3.228-fcoe
fci() { 
case $1 in
	2) arg=$fcoe2;;
	3) arg=$fcoe3;;
	*) arg=$1;;
esac
fcoeadm -i $arg; }
fcs() { 
case $1 in
	2) arg=$fcoe2;;
	3) arg=$fcoe3;;
	*) arg=$1;;
esac
fcoeadm -S $arg; }
fcl() { 
case $1 in
	2) arg=$fcoe2;;
	3) arg=$fcoe3;;
	*) arg=$1;;
esac
fcoeadm -l $arg; }
fcr() { 
case $1 in
	2) arg=$fcoe2;;
	3) arg=$fcoe3;;
	*) arg=$1;;
esac
fcoeadm -r $arg; }
fct() { 
case $1 in
	2) arg=$fcoe2;;
	3) arg=$fcoe3;;
	*) arg=$1;;
esac
fcoeadm -t $arg; }
gdo() {
get-dcb-oper.sh $1
}
gdp() {
get-dcb-peer.sh $1
}
gdc() {
get-dcb-config.sh $1
}

set -o vi
shopt -s histappend
#PS1='\h\w\$ '
export GIT_AUTHOR_NAME="Kevin Ragan"
export GIT_AUTHOR_EMAIL="kevinx.ragan@intel.com"
export GIT_COMMITER_NAME=root
export GIT_COMMITER_EMAIL="<$USER@$HOSTNAME>"

## MEDUSA BEGIN ##
## Automatically generated - DO NOT edit between these comment lines.

if [ "${MEDUSA_ENV:-''}" != "yup" ]; then
        MEDUSA_MLTT_INSTALL_DIR="/opt/medusa_labs/test_tools/"
        export MEDUSA_MLTT_INSTALL_DIR
        PATH="${MEDUSA_MLTT_INSTALL_DIR}bin:${PATH}"
        export PATH
        MEDUSA_MLM_ADMIN_CFG="/opt/medusa_labs/test_tools/config/MedusaTools.cfg"
        export MEDUSA_MLM_ADMIN_CFG
        MEDUSA_MLM_PRODUCT=1/1
       export MEDUSA_MLM_PRODUCT
        MEDUSA_ENV="yup"
        export MEDUSA_ENV
fi

type pain > /dev/null 2>&1
if [ $? -ne 0 ]; then
    PATH="${MEDUSA_MLTT_INSTALL_DIR}bin:${PATH}"
fi

## MEDUSA END ##
