#!/usr/bin/awk -f
BEGIN { RS = "\n\n"; FS = "\n"; ORS = RS ; OFS = FS }
function field(name,   i,f) {
    for (i = 1; i <= NF; i++) {
        split($i, f, ": *")
        if (f[1] == name)
            return f[2]
    }
    return ""
}

function fieldw(name, value,  i ){
    for (i=1; i<=NF; i++) 
         printf ( "%s\n", $i )
         #print $i
    print name ": " value ORS
}

{ print ("ssh " field('host') "'uname -r'") }
#{ print field("host") }

#{ fieldw("kernel", 2.4.29) }
