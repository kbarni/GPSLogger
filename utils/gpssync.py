#!/usr/bin/python3

from datetime import datetime, timedelta
from dateutil import tz
from pyproj import Transformer
import argparse
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

parser = argparse.ArgumentParser(description='Correction de géolocation des fichiers CSV utilisant un log PPK/RTK.')
parser.add_argument('--xarm', help='Lateral distance between antenna and camera (positive=right)', action='store', default=0, type=float)
parser.add_argument('--yarm', help='Axial distance between antenna and camera (positive=ahead)', action='store', default=0, type=float)
parser.add_argument('gpsfile', metavar='PPK log', help='PPK corrected log file',type=str, nargs=1, action='store')
parser.add_argument('camfile', metavar='Camera log', help='Raw camera log file',type=str, nargs='+', action='store')

args = parser.parse_args()

gpsfile = open(args.gpsfile[0])

ppkdata = [line.split() for line in gpsfile.readlines()]

ppkdecode = []

WGS2LAM = Transformer.from_crs("EPSG:4326", "EPSG:2154", always_xy = True)
LAM2WGS = Transformer.from_crs("EPSG:2154", "EPSG:4326", always_xy = True)

xprev = 0
yprev = 0
xref = 0
yref = 0

xarm = args.xarm # lateral distance between antenna and camera
yarm = args.yarm # 

print("Using camera/antenna distance {}/{}m".format(xarm,yarm))
print("------------------------")
print("Reading PPK file "+args.gpsfile[0])

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
print()
print("------------------------")

for filename in args.camfile:
    camfile = open(filename)
    corrfile = open(filename[:-4]+"_2.txt","w")
    print("Processing file "+filename)

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
    print()