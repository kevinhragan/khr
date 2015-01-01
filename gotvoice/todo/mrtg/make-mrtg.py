#! /usr/bin/python 
# create an mrtg.cfg file

### Host Lists ###
#Most machines should need to be added to one of the first three lists
Asterisk = [ "fig", "ptah", "voipa", "voipb" ]
Most = Asterisk + [ "cairo", "hathor", "host1", "mail", "nekhebet", "newpop2", "trial", "uadjet"  ]     
All = Most + [ "heket", "isis", "khufu", "osiris" "osiris2", "ramses", "seth", "shu", "www" ]
Eth1 = [ "fig", "osiris", "osiris2" ]

print """
EnableIPv6: no
WorkDir: /var/www/htdocs/dirname_mrtg
Options[_]: growright, bits
Maxbytes[_]: 12500000

### Targets"""
 
### NETWORK ###

## network stats on eth0
for host in All:
    hostnet=host + "-eth0"
    print """
Target[%s]: `/usr/local/bin/mrtg-net.sh %s eth0`
Title[%s]: %s
PageTop[%s]: <h1> %s Stats</h1>
""" % ( hostnet, host, hostnet, hostnet.upper(), hostnet, hostnet.upper() ) 

## network stats on eth1
for host in [ "fig" "osiris", "osiris2", ]:
    hostnet=host + "-eth1"
    print """
Target[%s]: `/usr/local/bin/mrtg-net.sh %s eth1`
Title[%s]: %s
PageTop[%s]: <h1> %s Stats</h1>
""" % ( hostnet, host, hostnet, hostnet.upper(), hostnet, hostnet.upper() ) 
 

### ASTERISK ###

print """
### asterisk check defaults
#Maxbytes[_]: 300
ThreshMinI[_]: 0
ThreshMinO[_]: 0
YLegend[_]: SIP Calls
LegendI[_]: Total
LegendO[_]: Inbound
Legend1[_]: Total
Legend2[_]: Inbound
ShortLegend[_]: Calls
Options[_]: gauge growright 

"""


for host in Asterisk:
    hostservice= host + "_asterisk"
    print """
Target[%s]: `/usr/local/bin/mrtg-asterisk.sh %s`
Title[%s]: %s CALLS
PageTop[%s]: <h1> %s Call Stats</h1>
""" % ( hostservice, host, hostservice, host.upper(), hostservice.upper(), host.upper() ) 
 

### LOAD AVERAGE ***

print """
 ### load average check defaults

YLegend[_]: Processes 
LegendI[_]: Load Average
LegendO[_]:
Legend1[_]: Load Average
ShortLegend[_]: Processes
YTicsFactor[_]: 0.01
Factor[_]: 0.01
Options[_]: gauge growright noo
"""

for host in Most:
    hostservice= host + "_cpu"
    print """
Target[%s]: `/usr/local/bin/mrtg-load.sh %s`
Title[%s]: %s LA
PageTop[%s]: <h1> %s Load Average/h1>
""" % ( hostservice, host, hostservice, host.upper(), hostservice.upper(), host.upper() ) 
 

### CPU BUSY ###
print \
"""# cpu check defaults
YLegend[_]: Percent
LegendI[_]: CPU Busy
LegendO[_]: IO Waiting
Legend1[_]: Percent Busy (Kernel and User)
Legend2[_]: Percent Waiting Block Devices
ShortLegend[_]: %
YTicsFactor[_]: 
Factor[_]: 
Options[_]: gauge growright 
"""

for host in Most:
    hostservice= host + "_cpu2"
    print \
"""Target[%s]: `/usr/local/bin/mrtg-cpu.sh %s`
Title[%s]: %s CPU
PageTop[%s]: <h1> %s CPU Usage/h1>
""" % ( hostservice, host, hostservice, host.upper(), hostservice.upper(), host.upper() ) 
 

### PAGE FAULTS ###

print """ 
# page fault check defaults
YLegend[_]: faults/second
YTicsFactor[_]: 0.01   
Factor[_]: 0.01   
LegendI[_]: Major Page Faults
Legend1[_]: Page Faults resulting in I/O
ShortLegend[_]:  faults/s
Options[_]: gauge growright noo
"""
for host in Most:
    hostservice= host + "_pagef"
    print """
Target[%s]: `/usr/local/bin/mrtg-pagef.sh %s`
Title[%s]: %s Memory Paging
PageTop[%s]: <h1> %s Memory Paging</h1> """ % ( hostservice, host, hostservice, host.upper(), hostservice.upper(), host.upper() ) 
 

### DISK ###
# leaving this a a literal for now

print """
# disk check defaults
# disk designations are disk numbers from sar
YLegend[_]: bytes/second
YTicsFactor[_]:    
Factor[_]:    
LegendI[_]: bytes READ per second
LegendO[_]: bytes WRITTEN per second
Legend1[_]: 
ShortLegend[_]: B/s
Options[_]: gauge growright 

Target[hathor_root_disk]: `/usr/local/bin/mrtg-disk.sh hathor 253-0`
Title[hathor_root_disk]: HATHOR Root Disk I/O
PageTop[hathor_root_disk]: <h1>HATHOR Root Disk I/O </h1>

Target[hathor_srv_disk]: `/usr/local/bin/mrtg-disk.sh hathor 253-1`
Title[hathor_srv_disk]: HATHOR NFS-shared Disk I/O
PageTop[hathor_srv_disk]: <h1>HATHOR NFS-shared Disk I/O </h1>

#Target[mail_disk]: `/usr/local/bin/mrtg-disk.sh mail `
#Title[mail_disk]: MAIL Disk I/O
#PageTop[mail_disk]: <h1>MAIL Disk I/O </h1>

Target[javabox_disk]: `/usr/local/bin/mrtg-disk.sh trial 9-0`
Title[javabox_disk]: JAVABOX Disk I/O
PageTop[javabox_disk]: <h1> JAVABOX Disk I/O </h1>

Target[host1_root_disk]: `/usr/local/bin/mrtg-disk.sh host1 9-0`
LegendI[_]: bytes READ per second
LegendO[_]: bytes WRITTEN per second
Legend1[_]: 
ShortLegend[_]: B/s
Options[_]: gauge growright 

Target[hathor_root_disk]: `/usr/local/bin/mrtg-disk.sh hathor 253-0`
Title[hathor_root_disk]: HATHOR Root Disk I/O
PageTop[hathor_root_disk]: <h1>HATHOR Root Disk I/O </h1>

Target[hathor_srv_disk]: `/usr/local/bin/mrtg-disk.sh hathor 253-1`
Title[hathor_srv_disk]: HATHOR NFS-shared Disk I/O
PageTop[hathor_srv_disk]: <h1>HATHOR NFS-shared Disk I/O </h1>

#Target[mail_disk]: `/usr/local/bin/mrtg-disk.sh mail `
#Title[mail_disk]: MAIL Disk I/O
#PageTop[mail_disk]: <h1>MAIL Disk I/O </h1>

Target[javabox_disk]: `/usr/local/bin/mrtg-disk.sh trial 9-0`
Title[javabox_disk]: JAVABOX Disk I/O
PageTop[javabox_disk]: <h1> JAVABOX Disk I/O </h1>

Target[host1_root_disk]: `/usr/local/bin/mrtg-disk.sh host1 9-0`
Title[host1_root_disk]: HOST1 Root Disk I/O                         
PageTop[host1_root_disk]: <h1> HOST1 Root Disk I/O </h1>

Target[host1_var_disk]: `/usr/local/bin/mrtg-disk.sh host1 9-1`
Title[host1_var_disk]: HOST1 Var Disk I/O                         
PageTop[host1_var_disk]: <h1> HOST1 Var Disk I/O </h1>

Target[newpop2_disk]: `/usr/local/bin/mrtg-disk.sh newpop2 3-0`
Title[newpop2_disk]: NEWPOP2 Disk I/O                     
PageTop[newpop2_disk]: <h1> NEWPOP2 Disk I/O </h1>

Target[voip_a_disk]: `/usr/local/bin/mrtg-disk.sh voipa 8-0`
Title[voip_a_disk]: VOIPA Disk I/O
PageTop[voip_a_disk]: <h1>VOIPA Disk I/O </h1>
"""
