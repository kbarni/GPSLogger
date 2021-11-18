#!/usr/bin/python3

import sys
from datetime import datetime, timedelta

def findClosest(sortedarray,val,minindex,maxindex):
    if maxindex-minindex < 2:
        return minindex;
    else:
        midindex = (maxindex+minindex) // 2
        if val<sortedarray[midindex][0]:
            return findClosest(sortedarray,val,minindex,midindex)
        else:
            return findClosest(sortedarray,val,midindex,maxindex)

if not len(sys.argv)==3:
    print("Error! incorrect number of arguments!\n\nUseage: ./gpssync.py [camerafile] [gpsfile]\nwhere camerafile is data.txt and gpsfile is ")

camfile = open(sys.argv[1])
gpsfile = open(sys.argv[2])

ppkdata = [line.split() for line in gpsfile.readlines()]
cameradata = [line.split() for line in camfile.readlines()]

ppkdecode = []
tzcorr = timedelta(hours=2)
for data in ppkdata:
    if data[0][0] == '%': 
        continue
    ts = datetime.strptime(data[1],'%H:%M:%S.%f')+tzcorr
    lat = float(data[2])
    lon = float(data[3])
    ppkdecode.append([ts,lat,lon])

corrfile=open(sys.argv[1][:-4]+"_2.txt","w")
print("Writing file "+sys.argv[1][:-4]+"_2.txt")
dmax = timedelta(seconds=2)

for data in cameradata:
    ts = datetime.strptime(data[3],'%H:%M:%S.%f')
    #lat = float(data[1])
    #lon = float(data[2])
    index = findClosest(ppkdecode,ts,0,len(ppkdecode))
    #print(ts,ppkdecode[index],ppkdecode[index+1])
    if ppkdecode[index+1][0]-ppkdecode[index][0]>dmax:
        print("Not correcting data...")
        corrfile.write("{}\t{}\t{}\t{}\t{}\t{}\t{}\n".format(data[0],data[1],data[2],data[3],data[4],data[5],data[6]))
        continue
    #print("Interpolating "+ts+" between "+ppkdecode[index+1][0]+" and "+ppkdecode[index+1][0])
    frac = (ts-ppkdecode[index][0])/(ppkdecode[index+1][0]-ppkdecode[index][0])
    latcorr = ppkdecode[index+1][1]*frac + ppkdecode[index][1]*(1-frac)
    loncorr = ppkdecode[index+1][2]*frac + ppkdecode[index][2]*(1-frac)
    corrfile.write("{}\t{}\t{}\t{}\t{}\t{}\t{}\n".format(data[0],latcorr,loncorr,data[3],data[4],data[5],data[6]))

corrfile.close()
    
#print(ppkdata)
