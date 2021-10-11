Write-Host "`nPPK conversion utility for UBX files`n(C)2021 Barna Keresztes / IMS Bordeaux`n"

if ($args.count -ne 2){
    Write-Host "Useage: correct.ps1 [input.ubx] [base.21o]`n"
    exit
}
$ubxfile=$args[0]
$obsfile=$ubxfile -replace ".ubx$",".obs"
$ppkfile=$ubxfile -replace ".ubx$",".pos"
$baseobs=$args[1]
$basenav=$baseobs -replace ".21o$",".21n"

Write-Host "Decoding $ubxfile to $obsfile..."
./convbin -r ubx -o $obsfile $ubxfile

Write-Host "Performing PPK Correction using $baseobs and $basenav"
./rnx2rtkp -k ./rtkpost.conf -o $ppkfile $obsfile $baseobs $basenav

Write-Host "Ready.`n"
