param([string]$Compiler = 'gcc')
$ErrorActionPreference = 'Stop'
$buildDir = Join-Path $PSScriptRoot 'build'
if (-not (Test-Path -LiteralPath (Join-Path $buildDir 'winmm.dll'))) { throw 'Run build.ps1 first.' }
$testExe = Join-Path $buildDir 'Blairwitch-Win64-Shipping.exe'
& $Compiler '-mcrtdll=ucrt' '-O2' '-Wall' '-Wextra' '-Werror' '-static-libgcc' "-I$buildDir" (Join-Path $PSScriptRoot 'tests\smoke.c') '-lwinmm' '-o' $testExe
if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
Copy-Item -LiteralPath $testExe -Destination (Join-Path $buildDir 'other-app.exe') -Force
Copy-Item -LiteralPath $testExe -Destination (Join-Path $buildDir 'Blairwitch-Win64-Shipping-Win7.exe') -Force
function Invoke-Probe([string]$Name, [string]$Expected, [string]$Existing) {
    $startInfo = [Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = Join-Path $buildDir $Name
    $startInfo.WorkingDirectory = $buildDir
    $startInfo.Arguments = $Expected
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $startInfo.EnvironmentVariables.Remove('OPENSSL_ia32cap')
    if ($Existing) { $startInfo.EnvironmentVariables['OPENSSL_ia32cap'] = $Existing }
    $probe = [Diagnostics.Process]::Start($startInfo)
    $stdout = $probe.StandardOutput.ReadToEnd()
    $stderr = $probe.StandardError.ReadToEnd()
    $probe.WaitForExit()
    if ($probe.ExitCode -ne 0) { throw "Probe $Name failed ($($probe.ExitCode)): $stderr $stdout" }
    Write-Output "$Name : $($stdout.Trim())"
}
Invoke-Probe 'Blairwitch-Win64-Shipping.exe' ':~0x20000000' ''
Invoke-Probe 'Blairwitch-Win64-Shipping-Win7.exe' ':~0x20000000' ''
Invoke-Probe 'other-app.exe' 'ABSENT' ''
Invoke-Probe 'Blairwitch-Win64-Shipping.exe' ':~0x12345678' ':~0x12345678'
Write-Output 'All native smoke tests passed.'
