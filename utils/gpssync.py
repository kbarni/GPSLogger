#!/usr/bin/python3

import sys
from datetime import datetime, timedelta
from dateutil import tz
from pyproj import Transformer
import math

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
    print("Error! incorrect number of arguments!\n\nUseage: ./gpssync.py [gpsfile] [camerafiles]\nwhere camerafile is data.txt and gpsfile is ")

gpsfile = open(sys.argv[1])

ppkdata = [line.split() for line in gpsfile.readlines()]

ppkdecode = []

WGS2LAM = Transformer.from_crs("EPSG:4326", "EPSG:2154", always_xy = True)
LAM2WGS = Transformer.from_crs("EPSG:2154", "EPSG:4326", always_xy = True)

xprev = 0
yprev = 0
xref = 0
yref = 0

xarm = -1 # lateral distance between antenna and camera
yarm = 1.6 # 

print("Reading PPK file...")

for data in ppkdata:
    if data[0][0] == '%': 
        continue
    # Get timestamp in UTC
    print(data[1],end='\r')
    ts = datetime.strptime(data[0]+" "+data[1],'%Y/%m/%d %H:%M:%S.%f').replace(tzinfo=tz.gettz('UTC'))
    # Get position in Lambert-93
    lat = float(data[2])
    lon = float(data[3])
    x,y = WGS2LAM.transform(lon,lat)
    # correct camera position
    # get heading
    heading = math.atan2(y-yref,x-xref)
    if (y-yprev)*(y-yprev)+(x-xprev)*(x-xprev) > 2 : 
        yref = yprev
        xref = xprev
        yprev = y
        xprev = x
    xcam = x + xarm * math.sin(heading) + yarm * math.cos(heading)
    ycam = y - xarm * math.cos(heading) + yarm * math.sin(heading)
    ppkdecode.append([ts,xcam,ycam,heading])

dmax = timedelta(seconds=2)

xcorr = ycorr = 0
curyear=ppkdecode[0][0].year
curmonth=ppkdecode[0][0].month
curday=ppkdecode[0][0].day

for filename in sys.argv[2:]:
    camfile = open(filename)
    corrfile = open(filename[:-4]+"_2.txt","w")
    print("\nProcessing file "+filename)

    cameradata = [line.split() for line in camfile.readlines()]
    corrfile.write("Image\tLat\tLon\tTimestamp\tParcelle\tSpe\tDist\tX_Lambert\tY_Lambert\n")
    for data in cameradata:
        if len(data)<7:continue
        print(data[3],end='\r')
        ts = datetime.strptime(data[3],'%H:%M:%S.%f').replace(year=curyear,month=curmonth,day=curday,tzinfo=tz.gettz('CET'))
        #lat = float(data[1])
        #lon = float(data[2])
        index = findClosest(ppkdecode,ts,0,len(ppkdecode))
        #print(ts,ppkdecode[index],ppkdecode[index+1])
        if ppkdecode[index+1][0]-ppkdecode[index][0]>dmax:
            print("\nNot correcting data...")
            xcorr,ycorr=WGS2LAM.transform(float(data[2]),float(data[1]))
            corrfile.write("{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\n".format(data[0],data[1],data[2],data[3],data[4],data[5],data[6],xcorr,ycorr))
            continue
        #print("Interpolating "+ts+" between "+ppkdecode[index+1][0]+" and "+ppkdecode[index+1][0])
        frac = (ts-ppkdecode[index][0])/(ppkdecode[index+1][0]-ppkdecode[index][0])
        xcorr = ppkdecode[index+1][1]*frac + ppkdecode[index][1]*(1-frac)
        ycorr = ppkdecode[index+1][2]*frac + ppkdecode[index][2]*(1-frac)
        loncorr,latcorr = LAM2WGS.transform(xcorr,ycorr)
        corrfile.write("{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\n".format(data[0],latcorr,loncorr,data[3],data[4],data[5],data[6],xcorr,ycorr))

    corrfile.close()
    camfile.close()
