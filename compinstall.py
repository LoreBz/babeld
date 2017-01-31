import os
cmd = 'make clean; make'
print cmd
os.system(cmd)

cmd = 'su -c \'make install\''
print cmd
os.system(cmd)

cmd = "sh cscoper.sh; make clean"
print cmd
os.system(cmd)
