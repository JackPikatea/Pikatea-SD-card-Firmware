$prefixArray = @{
    keyboard = 0x100;
    consumer = 0x200;
}
$rawValues = Import-Csv $PSScriptRoot\tmf_raw.csv | Sort-Object api

foreach ($entry in $rawValues) {    
    $entry.value = [int]$entry.value + $prefixArray[$entry.api];        
}

$outFile = "";
foreach ($entry in $rawValues) {
    $outFile += "$($entry.key)=$($entry.value)`r`n";    
}
$outFile |  Out-File $PSScriptRoot\TMF.txt -Encoding utf8 -NoNewline