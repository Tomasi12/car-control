<#
.SYNOPSIS
  Collects the planted markers from the sources into expected_findings.csv.
.DESCRIPTION
  Every planted issue is marked in the code on the line where it happens:
    /* DEFECT(category): ... */   run-time error an analyzer should report
    /* RULE(category): ... */     coding-rule violation an analyzer should report
    /* SAFE(category): ... */     correct code; reporting it is a false positive
  The CSV is the ground truth for comparing analyzers (SCA, clang-tidy, cppcheck, ...).
.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\tools\expected_findings.ps1
#>
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$pattern = '/\*\s*(DEFECT|RULE|SAFE)\(([a-z_]+)\):\s*(.*?)\s*\*/'

$rows = foreach ($f in Get-ChildItem (Join-Path $root 'src'), (Join-Path $root 'include') -Include *.c, *.h -Recurse) {
    $rel = $f.FullName.Substring($root.Length + 1).Replace('\', '/')
    $n = 0
    foreach ($line in Get-Content $f.FullName) {
        $n++
        $m = [regex]::Match($line, $pattern)
        if ($m.Success) {
            [pscustomobject]@{
                file        = $rel
                line        = $n
                kind        = $m.Groups[1].Value
                category    = $m.Groups[2].Value
                description = $m.Groups[3].Value
            }
        }
    }
}

$rows = $rows | Sort-Object kind, file, line
$out = Join-Path $root 'expected_findings.csv'
$rows | Export-Csv -NoTypeInformation -Encoding UTF8 $out
$rows | Group-Object kind | ForEach-Object { '{0,-7} {1}' -f $_.Name, $_.Count }
Write-Host "Wrote $out ($($rows.Count) markers)"
