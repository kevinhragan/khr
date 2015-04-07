man dmesg -n8
dmesg --help
fg
aptitude install rsyslog-doc
sudo aptitude install rsyslog-doc
jobs
xlock
scp -r mango:/usr/share/doc/rsyslog-doc ~/tmp
ls -l /tmp
ls -l ~/tmp
ls -trl ~/tmp
scp -r mango:/usr/share/doc/rsyslog-doc ~/tmp
ls -trl ~/tmp
#scp -r mango:/usr/share/doc/rsyslog-doc ~/tmp
xlock
vi provision.py
commit provision.py
git commit provision.py
vi provision.py
git commit provision.py 
git push
pylint -E provision.py 
git remote -v
xlock
grep git ~/commands 
vi ~/commands 
vi provision.py
git pull
git diff
vi provision.py
pylint -E provision.py 
vi provision.py
pylint -E provision.py 
git commit provision.py
git push
vi provision.py
ping mango
xlock
git status
git diff
git commit 
git commit provision.py 
git push
less ~/commands 
vi ~/commands 
vi /etc/ntp.conf 
sudo vi /etc/ntp.conf 
grep ntp provision.py
ntpd -q -g
sudo ntpd -q -g
sudo ntpd -q -g -v
sudo ntpd -q -g -d
sudo ntpd -q -g 
sudo ntpdate
sudo ntpdate corpad.glb.intel.com
date
ping corpad.glb.intel.com
date
git commit
git commit provision.py 
git push
vi provision.py
jobs
pgrep vi 
xlock
rx atlas
xl
ssh 10.166.48.254
ssh 10.166.48.251
ssh 10.166.48.250
ssh 10.166.48.247
less provision.py
cd
lftp mango
cd 
cd notes
vi salt
xlock
sudo salt-call -l debug state.highstate
xlock
vi salt
sudo  salt-call state.highstate test=True
salt --version
salt-minion --version
sudo  salt-call state.highstate test=True
sudo salt-call state.highstate 
cat /etc/hosts
xl
xlock
xl
xlock
xl
xlock
cd ..
cd git/provision/
ls
grep ntp tool/execute/provision.py
dig corpad.glb.intel.com
cat /etc/resolv.conf
xlock
cat /etc/resolv.conf 
grep intel.corp tool/execute/provision.py
vi  tool/execute/provision.py
git status
git commit status
git commit tool/execute/provision.py 
git push
dig LIN-ND2-066
host LIN-ND2-066
host LIN-ND2-066.jf.intel.com
host LIN-ND2-067.jf.intel.com
host LIN-ND2-064.jf.intel.com
host LIN-ND2-065.jf.intel.com
host 10.166.48.65
host 10.166.48.66
host 10.166.48.67
host 10.166.48.68
grep 068 ~/tmp/dns-hosts5
grep 067 ~/tmp/dns-hosts5
host 10.166.48.68
host 10.166.48.64
host 10.166.48.65
host 10.166.48.66
host 10.166.48.67
host 10.166.48.68
host lin-nd2-066.jf.intel.com.
host lin-nd2-067.jf.intel.com.
host lin-nd2-065.jf.intel.com.
host lin-nd2-068.jf.intel.com.
host lin-nd2-069.jf.intel.com.
xlock
cat /etc/resolv.conf 
dig jf.intel.com ns
#dig @10.19.1.4 #
cat /etc/resolv.conf
xlock
host 10.166.48.100
for ip in  10.166.48.1..255; do host $ip; done |less
for ip in  10.166.48.{1..255}; do host $ip; done |less
cd tool/execute/
vi provision.py
cd
grep -i prox commands 
ls
less jac.log.0
rm jac*
ls
rm 0*
ls bin
cd software/
tar xzf salt-2014.7.0.tar.gz 
cd salt-2014.7.0/
ls
less README.rst 
less setup.py 
xlock
ls
less PKG-INFO 
cd ..
scp salt-2014.7.0.tar.gz n98:
xlock
cat ~/notes/salt
xlock
hostname
ifconfig
su -
hostname
sudo su -
cd ~/git
cd provision/tool/execute/
git status
vi provision.py
ls ~root
fg
cp provision.py LIN-ND-098:~/provision/tools/execute
scp provision.py LIN-ND-098:~/provision/tools/execute
scp provision.py LIN-ND3-098:~/provision/tools/execute
scp provision.py LIN-ND3-098:~/provision/tool/execute
git push
git pull
ls -l provision.py
git commit -a
git push
cd ..
cd
cat tmp/dns-hosts5
cp tmp/dns-hosts5 ~/lin-hosts
vi ~/lin-hosts
awk -F, '{print $2, 5}' ~/lin-hosts
awk -F, '{print $2, $5}' ~/lin-hosts
awk -F, '{print $2, $5}' ~/lin-hosts >~/lin-hosts2
vi ~/lin-hosts2
awk [print $1} ~/lin-hosts2 > ~/lin-hosts3
awk '{print $1}' ~/lin-hosts2 > ~/lin-hosts3
vi lin-hosts3
while read host; do if ping -c 1 $host; then echo $host; done < ~/lin-hosts3
while read host; do if ping -c 1 $host; then echo $host;fi; done < ~/lin-hosts3
while read host; do if ping -c 1 $host; then echo $host;fi; done < ~/lin-hosts3 > ~lin-hosts-alive
ls -ltr
mv \~lin-hosts-alive ~/lin-hosts-alive
wc -l lin-hosts-alive 
less lin-hosts-alive 
vi lin-hosts-alive
#while read host; do scp key root@$host:.ssh/authorized_keys; done < ~lin-hosts-alive
sudo cp ~root/.ssh/authorized_keys key
vi key
sudo vi key
sudo +w key
sudo chmod +w key
cat key
sudo chmod +r key
sudo su -
xlock
ls -ltr
less lin-hosts3
less lin-hosts-alive 
vi tmp/hosts
cat tmp/hosts
host LIN-ND1-032
host LIN-ND1-034
host LIN-ND1-064
host LIN-ND1-066
host LIN-ND1-067
host LIN-ND1-096
host LIN-ND1-128
host LIN-ND1-160
ping LIN-ND2-064
host -x 10.166.48.160
host  10.166.48.160
host  10.166.48.64
host  lin-nd1-064.jf.intel.com
ping  lin-nd1-064.jf.intel.com
ping  #lin-nd1-064.jf.intel.com
ping kraganx
ping khr
xlock
cd git/provision/
cd tool/execute/
less provision.py
#while read host; do scp key root@$host:.ssh/authorized_keys; done < ~lin-hosts-alive
cd
ls key
for host in 10.166.48.64; 10.166.48.66
10.166.48.67
10.166.48.96
10.166.48.97
10.166.48.99
10.166.48.101
10.166.48.104
10.166.48.106
10.166.48.128
10.166.48.130
10.166.48.133
10.166.48.134
10.166.48.137
10.166.48.180
for host in '10.166.48.64
10.166.48.66
10.166.48.67
10.166.48.96
10.166.48.97
10.166.48.99
10.166.48.101
10.166.48.104
10.166.48.106
10.166.48.128
10.166.48.130
10.166.48.133
10.166.48.134
10.166.48.137
10.166.48.180
10.166.48.196'; do ping -c 1 $host;done
ping -c 10.166.48.64
ping -c1 10.166.48.64
ping -c1 10.166.48.66
ping -c1 10.166.48.67
ping -c1 10.166.48.96
ping -c1 10.166.48.97
ping -c1 10.166.48.99
ping -c1 10.166.48.101
ping -c1 10.166.48.104
ping -c1 10.166.48.106
ping -c1 10.166.48.128
sudo su 
sudo ls
alias
(localt&)
history -w
sudo aptitude install smbclient 
smbclient --help
smbclient -L mango
smbclient -L mango -U kraganx -P khr1khr2
xlock
modinfo netcat
xlock
cd git/provision/tool/execute/
vi provision.py
xlock
man dmesg -n8
dmesg --help
fg
aptitude install rsyslog-doc
sudo aptitude install rsyslog-doc
jobs
xlock
scp -r mango:/usr/share/doc/rsyslog-doc ~/tmp
ls -l /tmp
ls -l ~/tmp
ls -trl ~/tmp
scp -r mango:/usr/share/doc/rsyslog-doc ~/tmp
ls -trl ~/tmp
#scp -r mango:/usr/share/doc/rsyslog-doc ~/tmp
xlock
vi provision.py
commit provision.py
git commit provision.py
vi provision.py
git commit provision.py 
git push
pylint -E provision.py 
git remote -v
xlock
grep git ~/commands 
vi ~/commands 
vi provision.py
git pull
git diff
vi provision.py
pylint -E provision.py 
vi provision.py
pylint -E provision.py 
git commit provision.py
git push
vi provision.py
ping mango
xlock
git status
git diff
git commit 
git commit provision.py 
git push
less ~/commands 
vi ~/commands 
vi /etc/ntp.conf 
sudo vi /etc/ntp.conf 
grep ntp provision.py
ntpd -q -g
sudo ntpd -q -g
sudo ntpd -q -g -v
sudo ntpd -q -g -d
sudo ntpd -q -g 
sudo ntpdate
sudo ntpdate corpad.glb.intel.com
date
ping corpad.glb.intel.com
date
git commit
git commit provision.py 
git push
vi provision.py
jobs
pgrep vi 
xlock
rx atlas
xl
ssh 10.166.48.254
ssh 10.166.48.251
ssh 10.166.48.250
ssh 10.166.48.247
less provision.py
cd
lftp mango
cd 
cd notes
vi salt
xlock
sudo salt-call -l debug state.highstate
xlock
vi salt
sudo  salt-call state.highstate test=True
salt --version
salt-minion --version
sudo  salt-call state.highstate test=True
sudo salt-call state.highstate 
cat /etc/hosts
xl
xlock
xl
xlock
xl
xlock
cd ..
cd git/provision/
ls
grep ntp tool/execute/provision.py
dig corpad.glb.intel.com
cat /etc/resolv.conf
xlock
cat /etc/resolv.conf 
grep intel.corp tool/execute/provision.py
vi  tool/execute/provision.py
git status
git commit status
git commit tool/execute/provision.py 
git push
dig LIN-ND2-066
host LIN-ND2-066
host LIN-ND2-066.jf.intel.com
host LIN-ND2-067.jf.intel.com
host LIN-ND2-064.jf.intel.com
host LIN-ND2-065.jf.intel.com
host 10.166.48.65
host 10.166.48.66
host 10.166.48.67
host 10.166.48.68
grep 068 ~/tmp/dns-hosts5
grep 067 ~/tmp/dns-hosts5
host 10.166.48.68
host 10.166.48.64
host 10.166.48.65
host 10.166.48.66
host 10.166.48.67
host 10.166.48.68
host lin-nd2-066.jf.intel.com.
host lin-nd2-067.jf.intel.com.
host lin-nd2-065.jf.intel.com.
host lin-nd2-068.jf.intel.com.
host lin-nd2-069.jf.intel.com.
xlock
cat /etc/resolv.conf 
dig jf.intel.com ns
#dig @10.19.1.4 #
cat /etc/resolv.conf
xlock
host 10.166.48.100
for ip in  10.166.48.1..255; do host $ip; done |less
for ip in  10.166.48.{1..255}; do host $ip; done |less
cd tool/execute/
vi provision.py
cd
grep -i prox commands 
ls
less jac.log.0
rm jac*
ls
rm 0*
ls bin
cd software/
tar xzf salt-2014.7.0.tar.gz 
cd salt-2014.7.0/
ls
less README.rst 
less setup.py 
xlock
ls
less PKG-INFO 
cd ..
scp salt-2014.7.0.tar.gz n98:
xlock
cat ~/notes/salt
xlock
hostname
ifconfig
su -
hostname
sudo su -
cd ~/git
cd provision/tool/execute/
git status
vi provision.py
ls ~root
fg
cp provision.py LIN-ND-098:~/provision/tools/execute
scp provision.py LIN-ND-098:~/provision/tools/execute
scp provision.py LIN-ND3-098:~/provision/tools/execute
scp provision.py LIN-ND3-098:~/provision/tool/execute
git push
git pull
ls -l provision.py
git commit -a
git push
cd ..
cd
cat tmp/dns-hosts5
cp tmp/dns-hosts5 ~/lin-hosts
vi ~/lin-hosts
awk -F, '{print $2, 5}' ~/lin-hosts
awk -F, '{print $2, $5}' ~/lin-hosts
awk -F, '{print $2, $5}' ~/lin-hosts >~/lin-hosts2
vi ~/lin-hosts2
awk [print $1} ~/lin-hosts2 > ~/lin-hosts3
awk '{print $1}' ~/lin-hosts2 > ~/lin-hosts3
vi lin-hosts3
while read host; do if ping -c 1 $host; then echo $host; done < ~/lin-hosts3
while read host; do if ping -c 1 $host; then echo $host;fi; done < ~/lin-hosts3
while read host; do if ping -c 1 $host; then echo $host;fi; done < ~/lin-hosts3 > ~lin-hosts-alive
ls -ltr
mv \~lin-hosts-alive ~/lin-hosts-alive
wc -l lin-hosts-alive 
less lin-hosts-alive 
vi lin-hosts-alive
#while read host; do scp key root@$host:.ssh/authorized_keys; done < ~lin-hosts-alive
sudo cp ~root/.ssh/authorized_keys key
vi key
sudo vi key
sudo +w key
sudo chmod +w key
cat key
sudo chmod +r key
sudo su -
xlock
ls -ltr
less lin-hosts3
less lin-hosts-alive 
vi tmp/hosts
cat tmp/hosts
host LIN-ND1-032
host LIN-ND1-034
host LIN-ND1-064
host LIN-ND1-066
host LIN-ND1-067
host LIN-ND1-096
host LIN-ND1-128
host LIN-ND1-160
ping LIN-ND2-064
host -x 10.166.48.160
host  10.166.48.160
host  10.166.48.64
host  lin-nd1-064.jf.intel.com
ping  lin-nd1-064.jf.intel.com
ping  #lin-nd1-064.jf.intel.com
ping kraganx
ping khr
xlock
cd git/provision/
cd tool/execute/
less provision.py
#while read host; do scp key root@$host:.ssh/authorized_keys; done < ~lin-hosts-alive
cd
ls key
for host in 10.166.48.64; 10.166.48.66
10.166.48.67
10.166.48.96
10.166.48.97
10.166.48.99
10.166.48.101
10.166.48.104
10.166.48.106
10.166.48.128
10.166.48.130
10.166.48.133
10.166.48.134
10.166.48.137
10.166.48.180
for host in '10.166.48.64
10.166.48.66
10.166.48.67
10.166.48.96
10.166.48.97
10.166.48.99
10.166.48.101
10.166.48.104
10.166.48.106
10.166.48.128
10.166.48.130
10.166.48.133
10.166.48.134
10.166.48.137
10.166.48.180
10.166.48.196'; do ping -c 1 $host;done
ping -c 10.166.48.64
ping -c1 10.166.48.64
ping -c1 10.166.48.66
ping -c1 10.166.48.67
ping -c1 10.166.48.96
ping -c1 10.166.48.97
ping -c1 10.166.48.99
ping -c1 10.166.48.101
ping -c1 10.166.48.104
ping -c1 10.166.48.106
ping -c1 10.166.48.128
sudo su 
sudo ls
alias
(localt&)
history -w
exec su - khr
rx 10.166.48.196
ping 10.166.48.196
rx 10.166.48.196
rx n99
ping n99
ping n97
jobs
xlock
ping n99
ssh-copy-id n99
rx n99
xl
xlock
man lvconvert
ssh 10.166.48.130
ping 10.166.48.130
ssh 10.166.48.130
man ufw
sudo vi /etc/hosts
less ~/.vimrc 
xk
xl
xlock
less /etc/hosts
rx n98
ssh lin-nd3-100
ping lin-nd3-100
host lin-nd3-100
ping lin-nd3-100
ssh-copy-id LIN-ND3-100
vi /etc/hosts
ssh LIN-ND3-100
rx lin-nd3-100
rx lin-nd3-107
ping lin-nd3-107
rx 10.166.48.129
exit
rx lin-nd4-041
rx lin-nd4-141
ssh-copy-id lin-nd4-141
rx lin-nd4-141
exit
sudo ls
groups
grep sudo /etc/group
su - khr
groups
sudo ls
hostname
xlock
xl
import python
python
ls -ld junkdir/
ping ume
rx ume
rx lin-nd3-124
host lin-nd3-124
rx lin-nd3-125
host lin-nd3-125
rx lin-nd3-125
ping  lin-nd3-125
exit
rx lin-nd4-141
exit
rx ume
exit
rx ume
exit
ssh-copy-id lin-nd3-102
ip a
ping lin-nd3-102
rx lin-nd3-102
ip l
watch ip l
ip l
ip a
ifconfig -a
less /var/log/messages
sudo less /var/log/messages
dmesg |less
rx lin-nd3-102
ping n97
rx n97
rx atlas
rx kraganx@atlas
rx lin-nd4-129
ssh-copy-id lin-nd4-129
rx lin-nd4-129
reboot
rx lin-nd4-129
ping lin-nd4-129
rx lin-nd4-129
ping lin-nd4-129
rx lin-nd4-129
nmap lin-nd4-129
rx lin-nd4-129
rx mango
egrep 'bjm|jmo' /etc/passwd
rx mango
ssh root@mango
rx mango
rx ume
xlock
xt mango
hostname
grep xt .bash*
rx mango
ssh root@mango
xl
xlock
rx mango
vi scratch 
vi notes/kvm 
xlock
vi scratch 
cd notes
ls
grep prox *
vi scratch
cd ../
vi scratch
vi scratch 
xl
xlock
vi scratch 
ls
git status
git diff scratch
touch scratch
git diff scratch
git help
rm scratch
git checkout scratch
ls scratch 
cat scratch
git add -A
vi scratch
xl
lx
vi scratch
vi sc
vi scratch 
vi scratch
vi sratch
vi scratch
ssh-copy-id lin-nd3-107
rx lin-nd3-107
ping lin-nd3-107
rx lin-nd3-107
rx cherry
ssh-copy-id kraganx@cherry
ssh cherry
rx cherry
ping cherry
rx cherry
sed -i '131,132d' .ssh/known_hosts 
rx cherry
ping cherry
rx cherry
xl
useradd --help
rx cherry
rx lin-nd3-098
ssh-copy-id lin-nd-098
ssh-copy-id lin-nd3-098
rx lin-nd3-098
rx lin-nd3-100
sudo ls
alias
(localt&)
history -w
exec su - khr
sudo ls
xl
xlock
xl
ls -l /dev/disk/by-uuid/
ls /dev/disk
ls /dev/disk/by-id
ls -l /dev/disk/by-id
ls -ltr /etc
locate "*/login"
find /etc -mtime -1
sudo find /etc -mtime -1
man -k login
locate "*/nologin"
man nologin
man 5 nologin
xl
man -k badblock
man badblocks
man badblock
man badblocks
xl
dd if=/dev/zero of=/dev/sdc count=1 bs=1G
xl
cd git
ls
cd provision/
git remote -v
git status
git pull
lftp mango
xl
cd tool/execute/
ls
git pull
git push
#vi provision.py#
cd /etc
sudo git status
xl
cd ~/git/provision/tool/execute/
vi provision.py
pylint -E provision.py 
scp provision.py n98:/root/provision/tool/execute
scp provision.py lin-nd3-098:/root/provision/tool/execute
ssh-copy-id 
ssh-copy-id  lin-nd3-098
xlock
git commit -a
git push
xl
xlock
xl
xlock
xl
xlock
xl
cd ~/iso
ls
lftp linux-ftp
lftp linux-ftp:/pub/mirrors/fedora/linux/releases/21/Server/x86_64/Fedora-Server-DVD-x86_64-21.iso
lftp http://linux-ftp/pub/mirrors/fedora/linux/releases/21/Server/x86_64/Fedora-Server-DVD-x86_64-21.iso
lftp get http://linux-ftp/pub/mirrors/fedora/linux/releases/21/Server/x86_64/Fedora-Server-DVD-x86_64-21.iso
wget http://linux-ftp/pub/mirrors/fedora/linux/releases/21/Server/x86_64/Fedora-Server-DVD-x86_64-21.iso
set |grep PROX
set |grep prox
wget http://linux-ftp.jf.inte.com/pub/mirrors/fedora/linux/releases/21/Server/x86_64/Fedora-Server-DVD-x86_64-21.iso
#wget http://linux-ftp.jf.inte.com/pub/mirrors/fedora/linux/releases/21/Server/x86_64/Fedora-Server-DVD-x86_64-21.iso
export |grep prox
wget http://linux-ftp.jf.intel.com/pub/mirrors/fedora/linux/releases/21/Server/x86_64/Fedora-Server-DVD-x86_64-21.iso
wget http://linux-ftp.jf.intel.com/pub/mirrors/fedora/linux/releases/21/
wget http://linux-ftp.jf.intel.com/pub/mirrors/fedora/linux/releases/21/Server
wget http://linux-ftp.jf.intel.com/pub/mirrors/fedora/linux/releases/21/Server/x86_64
wget http://linux-ftp.jf.intel.com/pub/mirrors/fedora/linux/releases/21/Server/x86_64//Fedora-Server-DVD-x86_64-21.iso
lftp http://linux-ftp.jf.intel.com/pub/mirrors/fedora/linux/releases/21/Server/x86_64
wget http://linux-ftp.jf.intel.com/pub/mirrors/fedora/linux/releases/21/Server/x86_64/iso/Fedora-Server-DVD-x86_64-21.iso
#wget http://linux-ftp.jf.intel.com/pub/mirrors/fedora/linux/releases/21/Server/x86_64/iso/Fedora-Server-DVD-x86_64-21.iso
vi ~/commands 
ls
ls -ltr
rm Server x86_64  index.html Fedora-Server-DVD-x86_64-21.iso.1
ls
ls -ltr
cd ..
cd install
ls
find .
ls
cd ~/notes
ls
cat fedora
cd ~/git
ls
cd deploy
git pull
ls
cp {template,LIN-ND3-100}.linux.fc21.cfg 
vi LIN-ND3-100.linux.fc21.cfg 
vi template.linux.fc21.cfg 
cp {template,LIN-ND3-100}.linux.fc21.cfg 
vi LIN-ND3-100.linux.fc21.cfg 
scp LIN-ND3-100.linux.fc21.cfg mango:/srv/ftp/deploy
cd -
cd 
cd notes
grep ks fedora 
vi fedora
xlock
xl
cd ../tmp


less ~/git/provision/tool/execute/provision.py

yum-config-manager --nogpgcheck --add-repo ftp://10.166.48.35/fedora/Fedora-21-x86_64-Everything/Fedora-Everything.repo
#
# ks=ftp://10.166.48.35/deploy/LIN_ND#3-###.linux.fc##.cfg
xl
grep ks fedora 
grep ks= commands
grep ks= ../commands
grep ks ../commands
vi ../commands
cd 
cd notes
grep ks fedora 
cd ~/git
cd provision/tool/execute/
ls
less provision.py
cd -
xl
lx
xl
man lftp
grep ks= ~/notes/fedora 
ping LIN-ND3-100
xl
cd ../../../deploy/
git status
grep 20 template.linux.fc21.cfg 
grep 21 template.linux.fc21.cfg 
git commit -A
git commit -a
git push
git pull
rm LIN-ND3-100.linux.fc21.cfg 
git pull
git push
git status
cd /root/bin
cd /root
xl
xlock
xl
vi tmp/ansible
xl
xlock
xl
xlock
xl
lftp mango
xl
ls
git pull
ls
cp template.linux.rhel7.1.cfg LIN-ND4-129.linux.rhel7.1.cfg
cp template.linux.rhel7.1.cfg LIN-ND4-129.linux.rhel7.1.cfg
vi LIN-ND4-129.linux.rhel7.1.cfg
git status
git add LIN-ND4-129.linux.rhel7.1.cfg 
git commit -a
git push
lftp mango
scp LIN-ND4-129.linux.rhel7.1.cfg /srv/ftp/deploy
scp LIN-ND4-129.linux.rhel7.1.cfg mango:/srv/ftp/deploy
cd ~/iso
lftp linux-ftp
lftp http://linux-ftp/
lftp http://linux-ftp.jf.linux.com/
lftp http://linux-ftp.jf.intel.com
cd ../notes
grep ks= *
lx
xl
cd ../git/deploy
cp {template,107}.linux.rhel7.1.cfg 
ls
vi 107.linux.rhel7.1.cfg 
mv 107.linux.rhel7.1.cfg LIN-ND3-107.linux.rhel7.1.cfg 
xl
scp LIN-ND3-107.linux.rhel7.1.cfg mango:/srv/ftp/deploy
cd /usr/local/bin
ls
cd
ls
ls hosts
find . |grep L
find . |grep L[0-9]*
find . |grep L[0-9].*
ls
rm L7.xlsx 
rm linuxrc.log 
cat fix-git 
rm fix-git 
rm ApplicationSettings.xml 
rm DeviceTree.xml 
cat dummy
rm dummy 
ls
ls etc
rm new_hsd 
rm scratch
rm Testfile 
ls
cat lin-hosts
q
ls
mkdir dns
mv lin-hosts* dns
rm -r MRV_Cable_Pull/
ls nots
ls old
cat old/165
rm -r old
cat README 
rm README 
ls
ls testing
ls Webtools
rm -r Webtools/
ls admin
ls dumps
rm -r dumps/tcpdump.out 
cat index.html 
rm index.html 
ls
less jac.log.0
cat jira-comment 
rm jira-comment 
ls key
cat key
rm key
ls
vi 129
vi 107
cd notes
ls
rm L9-009 
rm 009 
rm salt
less install
cd ..
cd tests
cat 231770
xl
cat ../107
cd 
ls 
cat 107
get_pba.py 
cat 107
cat tests/231770
get_pba.py enp5s0f0
get_pba.py enp5s0f1
ethtool enp5s0f0
cat 107
cat tests/318989
cat tests/3188989 
cp tests/3188989 tests/318989
vi tests/381989
vi tests/318989
xl
cat tests/318989
cat tests/262058
vi notes/fabrics
cd git/provision/
less tool/execute/provision.py
cat tests/262058
cat `/tests/262058
cat ~/tests/262058
cat ~/107
xl
cd tool/execute/
vi provision.py
xl
pylint -E provision.py 
scp provision.py 10.166.48.107:
commit provision.py
git commit provision.py
git push
xl
apt-get update
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
sudo apt-get autoremove
df -h
ping 10.0.0.116
ping 10.0.0.117
ip r
nmap --help
ping -sn 10.0.0.1-254
nmap -sn 10.0.0.1-254
xl
vi 107
vi ~/107
cat ~/tests/333479
yum install mdadm
cd
vi 107
vi 129
cat ~/tests/333479

vi ~/tests/333479
xl
vi ~/tests/333479
cd tests
git status
rm .318989.swp 
git add -A
git commit -a
git push
xscreensaver --help
xscreensaver--demo 
xscreensaver --demo 
xscreensaver-demo 
xl
cat ~/tests/333479
cat ~/129
cd 
vi 129
cat tests/244884
cat notes/install
cat tests/244884
xl
man dump
xl
cat tests/167943
vi tests/167943
cat tests/167943
scp $! lin-nd3-107:
scp tests/167943  lin-nd3-107:
vi tests/167943
scp tests/167943  lin-nd3-107:
vi 107
less notes/install
cat 129
xl
grep pain commands
cd notes
grep bash *
grep bash_profile *
xl
cd 
cat tests/165489 
cat 107
vi 107
xl
vi 129
lx
xl
cat tests/165489
vi 107
cat 129
xl
vi ansible 
xl
vi ansible
xl
cat 129
cat 107
cat tests/209199
cat tests/209197
cat 107
vi tests/209197
grep -c vlan *
grep -c vlan tests/*
grep -C vlan tests/*
grep -C2 vlan tests/*
grep -C2 vlan tests/*|less
less tests/241629
vi tests/209197
xl
lx
xl
cat tests/209197
grep iperf commands 
grep iperf tests/*
xl
cat 107
cat tests/180787
ls -l tests/180787
vi tests/180787
xl
grep fcoe notes/install
ls
ls /etc/fcoe
uptime
xl
vi tests/187164
ls tests
git add -A
cd tests
git add -A
git commit -a
git push
git remote -v
ls
cd ~/git/provision/
ls
ls test_case/
git -v remote
git remote -v
git pull
cd -
ls 7164
ls 187164 
dc
bc
xl
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
df -h
date
xl
sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
cat 176995
last
hostname
less /var/log/messages
sudo less /var/log/messages
cd /var/log
ls
ls -ltr
sudo less auth.log
sudo less syslog
cd
xl
cat 107
locate ww
ls
vi admin/status/ww04
vi tests/180789
vi tests/180787
locate "*/git/provision/*tests"
locate "*/git/provision/*tests/180787"
xl
grep git remote commands 
cd git/provision
ls
git remote -v
xl
less ~/notes/install
cd
cat tests/187408
locate single-cable-pull
less ~/notes/install
lx
xl
less notes/install
xl
grep -i proxy commands 
grep -i proxy notes/*
grep -Ri proxy notes
grep proxy ~/.bash_profile 
vi notes/fedup
xl
locate dnsmasq
cd notes
grep -Ri dnsma .
less /etc/dnsmasq.conf 
apt-cache show syslinux
xl
vi ~/pw
lx
xl
ls /usr/local/bin/cd
cd 
cat admin/status/ww04
rm ww05 ww05 ww09 ww11 ww18 ww8
#rm ww05 ww05 ww09 ww11 ww18 ww8
cd admin/status/w
cd admin/status/
rm ww05 ww05 ww09 ww11 ww18 ww8
ls
cat ww04
xl
grep user ~/commands 
ssh casedanx@mango
cat /etc/group
xl
sudo apt-get update
sudo apt-get upgrade
xl
lx
ping 10.166.51.204
ping 10.166.51.203
gcd 
cd 
grep vlan commands 
xl
grep disktest commands
cd git/provision/tool/execute/
vi provision.py
xl
fg
scp provision.py lin-nd3-107:tmp
pylint -E provision.py 
pylint  provision.py 
xl
scp lin-nd3-107:tmp/provision.py .
git diff provision.py


vi provision.py
pylint  provision.py 
pylint -E  provision.py 
pylint --help
pylint -r no  provision.py 
vi provision.py
pylint -r no  provision.py 
vi provision.py
pylint -r no  provision.py 
ls
git commit provision.py 
git push
git status
cd ../..
git status
cd ~/tests
ls
ls *1808
cd 
cd notes
grep prio *
grep mount *
cat cgroup
vi cgroup
grep prio *
xl
cd ../
cd git/provision/tool/execute/
vi provision.py
locate provision.sh
cd ..
cd 
cd ~/notes
ls
cat cgroup 
xl
vi ../admin/excused
xl
vi ../admin/excused 
xl
cd 
cd hsd
mkdir 5535028
cd 5535028
ls
scp lin-nd3-107:tmp/messages .

xl
cd 
vi 100
xl
cd hsd/5535028/
scp lin-nd3-107:tmp/fcoemon.log .
ls
rm fcoemon.log 
xl
scp lin-nd3-107:tmp/fcoemon.log .
cd ..
cd ../notes
less install
xl
cd ..
cat 100
cat 102
cat 107
cat 100
vi admin/status/ww05
grep -i vmdq tests/*
xl
less git/provision/tool/execute/provision.py
xl
cd git
cd provision/
cd tool/execute/
git pull
vi provision.py
pylint -r no  provision.py 
vi provision.py
pylint -r no  provision.py 
xl
scp provision.py 10.166.48.143:
scp provision.py 10.166.48.107:
git diff
git commit -a provision.py 
git commit provision.py 
git diff
git diff provision.py
git push
lx
xl
cd ..
cd 
cd tests
ls 133341
cat 133341
cat ~/107
cat 133341
xl
lx
xl
vi 133341
xl
less ~/notes/install
vi 133341
xl
vi 133341
fg
git commit -a
git push
vi /tmp/tt
cd 
cd tmp
lftp mango
xl
xlock
ls /usr/local/bin
xl
cd 
vi commands 
xl
ping juniper
ssh root@juniper
vi commands
xlock
xl
vi commands
xl
xlock
xl
grep vlan commands
xl
cd git/provision
ls
vi tool/execute/provision.py
cd 
cat 129
vi 129
vi admin/status/ww05
vi admin/status/ww06
xlock
xl
cat 129
vi admin/status/ww06 
xl
less commands
xl
grep targets.dat commands 
xl
cat 107
vi 107
cat 107
xl
lx
xl
cat 107
cat ~/129
lftp http://linux-ftp
ping linux-ftp
ping linux-ftp.jf.intel.com
lftp http://linux-ftp.jf.intel.com
xl
tail commands 
less commands
cd tests/
vi 317193
xl
vi 317193
aptitude update
sudo aptitude update
sudo aptitude upgrade
cd
cd tmp
ls
rm Row1.ods 
ls -ltr
less 0001-fcoemon\ Disable\ FCoE\ interface\ if\ PFC\ not\ enabled.patch 
xl
sudo crontab -e
xl
cd ~/tests
cat 187408
../
cd ..
xl
cat admin/status/ww05 
cat admin/status/ww06
xlock
xl
xlock
xlocl
xlock
(localt&)
xlock
xl
(localt&)
xl
cat 107
cat 129
vi admin/status/ww07
xlodk
xlock
xl
grep 18408 *
xl
cd hsd
ls
cd 5535028/
ls
fill 0001-fcoemon\ Disable\ FCoE\ interface\ if\ PFC\ not\ enabled.patch 
file 0001-fcoemon\ Disable\ FCoE\ interface\ if\ PFC\ not\ enabled.patch 
man diff\
man diff
less 0001-fcoemon\ Disable\ FCoE\ interface\ if\ PFC\ not\ enabled.patch 
xl
ip r
ip a
vi /etc/network/interfaces
ping www.yahoo.com
ip l sh
ping juniper
sudo shutdown -h now
rx lin-nd2-062
rx lin-nd2-069
exit
rx mango
exit
(localt&)
rx 10.166.48.174
ssh-copy-id 10.166.48.174
rx 10.166.48.174
rx lin-nd3-125
ping lin-nd3-125
ssh-copy-id lin-nd3-125
rx lin-nd3-125
rx lin-nd3-107
rx lin-nd3-098
exit
rx 107
rx lin-nd3-107
rx lin-nd5-174
ping lin-nd3-107
rx lin-nd5-173
dig -x 10.166.48.173
ping lin-nd5-173
ping lin-nd5-173.jf.intel.com
ping lin-nd3-107
rx lin-nd3-107
rx lin-nd3-129
rx lin-nd4-129
exit
rx atlas
vi .ssh/config
rx atlas
ssh-copy-id atlas
rx atlas
/local
(localt&)
dmesg |grep mouse
grep rdesk commands 
rdesktop -u kraganx -p 7u8i&U*I -a 16 -g 1200x950 10.166.36.84
rdesktop -u kraganx -p '7u8i&U*I' -g 1200x950 10.166.36.84
ping 10.166.36.84
nmap 10.166.36.84
rdesktop -u kraganx -p '7u8i&U*I' -g 1200x950 10.166.36.84
nmap 10.166.36.84
rdesktop -u kraganx -p '7u8i&U*I' -g 1200x950 10.166.36.84
bg
(localt&)
xl
set |grep |PROX
set |grep PROX
cd hsd/5535028/
cp 0001-fcoemon\ Disable\ FCoE\ interface\ if\ PFC\ not\ enabled.patch lin-nd3-107:hsd/5535028/
scp 0001-fcoemon\ Disable\ FCoE\ interface\ if\ PFC\ not\ enabled.patch lin-nd3-107:hsd/5535028/
xl
cat tests/180787
cat `/tests/180787
cat ~/tests/180787
ls
cat 0001-fcoemon\ Disable\ FCoE\ interface\ if\ PFC\ not\ enabled.patch 
(localt&)
cd
xlock
xl
cat ~/tests/180787
less git/provision/tool/execute/provision.py
xl
cd git/provision/tool/execute/
vi provision.py
/pylint 
pylint -r no  provision.py 
pylint -r no  provision.py |grep -v line-too-long
vi provision.py
pylint -r no  provision.py |grep -v line-too-long
fg
scp provision.py 10.166.48.174:
git pull
xlock
xl
ls /root
hostname
ls /root
sudo ls /root/
vi ~/.bashrc
hostname
git pull
vi /home/kraganx/.bashrc
vi ~/.bashrc
ls
alias
git push
vi ~/.bashrc
vi ~/.bash_profile 
git commit provision.py 
git push
git commit -a
git push
git add -A
git push
cd 
vi admin/status/ww07
grep rdesk commands 
xlock
xl
virt-manager 
xl
lftp http://ftp-linux
ping fpt-linux
ping ftp-linux
ping linux-ftp
lftp http://linux-ftp/pub
lftp http://linux-ftp/
lynx http://linux-ftp/
ping linux-ftp
lynx ftp://linux-ftp/
host linux-ftp
lftp http://linux-ftp.jf.intel.com
xl
fg
bg
jobs
lftp http://linux-ftp.jf.intel.com
ls -ltr
cd iso
ls
cd ..
ls -l RHEL-7.1-20150129.0-Server-x86_64-dvd1.iso 
mv RHEL-7.1-20150129.0-Server-x86_64-dvd1.iso iso
vi notes/install
xl
xlock
xl
cat tests/187408
xl
vi tests/187408
xl
xlock
xl
vi tests/187408
fg
xl
grep mrv /etc/hosts
xl
cd git/provision/
vi tool/execute/provision.py
git commit -a
#vi tests/187408
cd
vi tests/187408
fg
#fg
vi tests/187408
xl
fg
xl
cd git
cd deploy
ls
vi junkfile
git push
git add -A
git commit junkfile
git push
git pull
git push
rm junkfile 
git add -A
git commit -a
git pull
git push
xl
cd
cd admin/status/
ls
ls -ltr
rm ww[234]*
ls
cat ww07
xl
ping fcoe
vi /etc/hosts
sudo cp /tmp/hosts /etc/hosts
ping fcoe
cd ~/notes
vi kvm
xl
cd 
vi 141
less notes/kvm
vi notes/kvm
aptitude install avahi
suod aptitude install avahi
sudo aptitude install avahi
aptitude install virt-manager
sudo aptitude install virt-manager
locate images
aptitude search ixgbevf
aptitude search ixgbe
fg
cd iso/
ls
scp RHEL-7.1-20150129.0-Server-x86_64-dvd1.iso lin-nd4-141:/var/libvirt/images/
scp RHEL-7.1-20150129.0-Server-x86_64-dvd1.iso lin-nd4-141:/var/lib/libvirt/images/
xl
cd ../notes
ls
vi notes/kvm
vi kvm 
xl
vi install
xl
cd ..
grep lldp commands
less  commands
echo ~
ls /home
sudo echo ~
sudo sh -c 'echo ~'
sudo sh -c 'echo ~khr'
xl
cd git
cd deploy
ls
git pull
git push
vi template.test-only.cfg
git add template.test-only.cfg 
git commit template.test-only.cfg
git push
git pull
date
rm template.test-only.cfg 
git rm template.test-only.cfg 
git commit -a
git push
git pull
sudo crontab -l
xl
grep proto ~/commands 
grep -B1 proto ~/commands 
cd ~/tmp
ls -ltr
xl
mount |grep media
dmesg 
fdisk -l /dev/sdb
sudo fdisk -l /dev/sdb
mount /dev/sdb2 /media/usb1
sudo mount /dev/sdb2 /media/usb1
sudo mount /dev/sdb2 /media/usb0/
ls /media/usb0/
cd /media/usb0/efi/boot/
ls
cd ..
sudo umount /dev/sdb2 /media/usb0/
sudo umount /dev/sdb2 
sudo mount /dev/sdb1 /media/usb0/
cd usb0
ls
df -h
sudo rm -r debian
cd ..
sudo umount /dev/sdb1
#sudo mount /dev/sdb1 /media/usb0/
man mount
sudo mount -w /dev/sdb1 /media/usb0/
mount 
mount |grep media
dmesg
dmesg --help
dmesg -H
fdisk -l /dev/sdb
sudo fdisk -l /dev/sdb
mount /dev/sdb1 /media/usb0
sudo mount /dev/sdb1 /media/usb0
cd /media/usb0
ls
df -h
sudo mount /dev/sdb1 /media/usb0
dmesg
sudo mount /dev/sdc1 /media/usb0
ls /media/usb0
cd /media/usb0
strings dellbios.exe 
strings dellbios.exe |less
ls
ls ~/tmp/*exe 
cp ~/tmp/*exe .
sudo cp ~/tmp/*exe .
ls
ls -ltr *exe
strings R710-020211C-1.exe 
strings R710-020211C-1.exe |less
sudo umount /media/usb0 
cd .
sudo umount /media/usb0 
cd ..
sudo umount /media/usb0 
xl
cd 
ls -ltr
cd tmp
ls -ltr

sudo mount /dev/sdc1 /media/usb0/
sudo umount /dev/sdc1 /media/usb0/
dmesg 
sudo umount /dev/sdb1 /media/usb0/
fdisk -l /dev/sdb
sudo fdisk -l /dev/sdb
mount |grep usb
sudo mount /dev/sdb1 /media/usb0/
cd /media/usb0
ls
ls del
ls dell
mv R710-020211C-1.exe dell
sudo mv R710-020211C-1.exe dell
mv dellbios.exe dell
sudo mv dellbios.exe dell
ls
rm Screenshot.png 
sudo rm Screenshot.png 
ls
rm bz-1129888-dracut.patch.txt 
sudo rm bz-1129888-dracut.patch.txt 
ls
dmesg |tail 20
dmesg |tail -20
mount |grep usb
df -h
#dd bs=400M if=/dev/sdb of=/dev/sdc
mount |grep sd
dd bs=400M if=/dev/sdb of=/dev/sdc
sudo dd bs=400M if=/dev/sdb of=/dev/sdc
xl
fdisk -l /dev/sdc
sudo fdisk -l /dev/sdc
man mkfs
man mkfs.vfat 
sudo fdisk -l /dev/sdb
man mkfs.fat
man fsck /dev/sdc1
sudo fsck /dev/sdc1
sudo fsck.msdos /dev/sdc1
sudo fsck.vfat /dev/sdc1
aptitude search biosdisk
aptitude search unetbootin
aptitude install unetbootin
sudo aptitude install unetbootin
sudo aptitude update
sudo aptitude install unetbootin
man unetbootin
unetbootin --help
aptitude install freedos
sudo aptitude install freedos
sudo aptitude search freedos
cfdisk /dev/sdc
sudo cfdisk /dev/sdc
fdisk -l /dev/sdc
sudo fdisk -l /dev/sdc
sudo unetbootin 
fdisk -l
sudo disk -l
sudo fdisk -l
sudo mkfs /dev/sdc1
sudo mkfs.msdos  /dev/sdc1
sudo fdisk -l
sudo unetbootin 
#sudo mount /dev/sdc1 /#
mkdir /mnt/verbatim
sudo mkdir /mnt/verbatim
sudo mount /dev/sdc1 /mnt/verbatim
sudo unetbootin 
xl
man git remote
cd git
cd cd ~/git
ls
cd ~
cd git
cd deploy
git remote get-url
git remote -v
git remote set-url origin dodo@feebok.org//path
git remote -v
git remote set-url origin kraganx@mango:/srv/git/scripts/deploy.git
git pull
git remote -v
sudo umount /dev/sdc1
xl
dmesg
sudo dd bs=400M if=/dev/sdb of=/dev/sdd
#sudo dd bs=400M if=/dev/sdb of=/dev/sdd
fdisk -l
sudo fdisk -l
#sudo dd bs=400M if=/dev/sdb of=/dev/sdd
fdisk -l
sudo fdisk -l
sudo dd bs=400M if=/dev/sdc of=~/win-usb.iso
bg
xl
grep screen ~/commands 
grep screen ~/notes/install
xl
dhclient --help
man dhclient
ip l
dhclient -dvn   -p4011
dhclient -dvn   -p4011 eth0
dhclient -dvn -p 4011 eth0
dhclient -d -v -n -p 4011 eth0
xl
man ufw
lx
xl
cd notes
cd ~/notes
ls -ltr
cat compile
ls ~
cat compile
vi compile
locate cisco.py
ls /home/khr/git/provision/tool/diagnostic/scripts/
cat tests/180787
vi notes/compile
xl
vi tests/180787
vi notes
vi notes/compile
ls
cd hsd/5535028/
ls
cat 0001-fcoemon\ Disable\ FCoE\ interface\ if\ PFC\ not\ enabled.patch 
xl
locate single-cable-pull
cd ..
cd 
locate kraganx
locate "*/kraganx"
cd git/provision/tool/tester/kraganx/
vi .bashrc 
locate .bashrc
grep PATH /home/khr/testing/users/kevin/.bashrc
grep PATH git/provision/tool/tester/kraganx/.bashrc
grep PATH ~/git/provision/tool/tester/kraganx/.bashrc
cd ../../execute/
less provision.py
grep disktest commands
cd
grep disktest commands
rx mango
vi scratch
xl
vi scratch
rx cherry
rx lin-nd4-141
rx 107
rx lin-nd3-107
rx lin-nd3-100
xl
rx lin-nd3-100
startx
ls 
ifconfig
ping juniper
startx
xstart
xinit
whereis startx
locate startx
hostname
i3
xinit
locate xinit
locate startx
cat /etc/X11/xinit/xinitrc 
aptitude search startx
aptitude search xinit
aptitude install xinit
sudo aptitude install xinit
startx
rx lin-nd4-140
ssh-copy-id lin-nd4-140
rx lin-nd4-140
ssh-copy-id lin-nd4-142
rx lin-nd4-142
exit
ping 10.166.48.67
ssh-copy-id 10.166.48.67
rx 10.166.48.67
rx 10.166.48.68
rx 48.125
rx 10.166.48.125
ping 10.166.48.125
rx 10.22.224.195
rx 10.166.51.6
(localt&)
rx ansible
rx atlas
rx lin-nd3-107
