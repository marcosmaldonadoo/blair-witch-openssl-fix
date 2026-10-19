param([string]$Compiler = 'gcc')
$ErrorActionPreference = 'Stop'
$repoRoot = $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build'
New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
$exports = @(Get-Content -LiteralPath (Join-Path $repoRoot 'src\exports.json') -Raw | ConvertFrom-Json)
$header = [System.Collections.Generic.List[string]]::new()
$definition = [System.Collections.Generic.List[string]]::new()
$assembly = [System.Collections.Generic.List[string]]::new()
$header.Add("#define EXPORT_COUNT $($exports.Count)")
$header.Add('static const char *const export_names[EXPORT_COUNT] = {')
foreach ($entry in $exports) {
    if ($entry.name) { $header.Add('    "' + $entry.name + '",') }
    else { $header.Add('    NULL,') }
}
$header.Add('};')
$header.Add('static const WORD export_ordinals[EXPORT_COUNT] = {')
foreach ($entry in $exports) { $header.Add("    $($entry.ordinal),") }
$header.Add('};')
$definition.Add('LIBRARY winmm')
$definition.Add('EXPORTS')
$assembly.Add('.text')
for ($i = 0; $i -lt $exports.Count; $i++) {
    $entry = $exports[$i]
    if ($entry.name) { $definition.Add("    $($entry.name)=proxy_$i @$($entry.ordinal)") }
    else { $definition.Add("    ordinal_$($entry.ordinal)=proxy_$i @$($entry.ordinal) NONAME") }
    # Preserve all Windows x64 parameter registers and the caller's stack args.
    # 136 bytes = 32 shadow + 32 integer + 64 vector + 8 alignment bytes.
    $assembly.Add(@"
.p2align 4
.globl proxy_$i
.seh_proc proxy_$i
proxy_${i}:
    subq `$136, %rsp
    .seh_stackalloc 136
    .seh_endprologue
    movq %rcx, 32(%rsp)
    movq %rdx, 40(%rsp)
    movq %r8, 48(%rsp)
    movq %r9, 56(%rsp)
    movdqu %xmm0, 64(%rsp)
    movdqu %xmm1, 80(%rsp)
    movdqu %xmm2, 96(%rsp)
    movdqu %xmm3, 112(%rsp)
    movl `$$i, %ecx
    call resolve_export
    movq 32(%rsp), %rcx
    movq 40(%rsp), %rdx
    movq 48(%rsp), %r8
    movq 56(%rsp), %r9
    movdqu 64(%rsp), %xmm0
    movdqu 80(%rsp), %xmm1
    movdqu 96(%rsp), %xmm2
    movdqu 112(%rsp), %xmm3
    addq `$136, %rsp
    jmp *%rax
.seh_endproc
"@)
}
[IO.File]::WriteAllLines((Join-Path $buildDir 'exports.generated.h'), $header, [Text.Encoding]::ASCII)
[IO.File]::WriteAllLines((Join-Path $buildDir 'proxy.def'), $definition, [Text.Encoding]::ASCII)
[IO.File]::WriteAllLines((Join-Path $buildDir 'trampolines.S'), $assembly, [Text.Encoding]::ASCII)
& $Compiler '-shared' '-Os' '-Wall' '-Wextra' '-Werror' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-nostdlib' '-Wl,--entry,DllMain,--dynamicbase,--nxcompat,--high-entropy-va,--no-insert-timestamp' "-I$buildDir" (Join-Path $repoRoot 'src\proxy.c') (Join-Path $buildDir 'trampolines.S') (Join-Path $buildDir 'proxy.def') '-lucrt' '-lkernel32' '-o' (Join-Path $buildDir 'winmm.dll')
if ($LASTEXITCODE -ne 0) { throw 'Compilation failed.' }
Write-Output "Built: $(Join-Path $buildDir 'winmm.dll')"
