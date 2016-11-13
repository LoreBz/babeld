import os
cmd = 'make'
print cmd
os.system(cmd)

cmd = 'su -c \'make install\''
print cmd
os.system(cmd)

cmd = "sh cscoper.sh"
print cmd
os.system(cmd)
