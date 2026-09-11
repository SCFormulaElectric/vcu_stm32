param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [string]$ToolchainBin = ''
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot

if ([string]::IsNullOrWhiteSpace($ToolchainBin)) {
    $tool = Get-ChildItem -Path 'C:\ST' -Filter 'arm-none-eabi-gcc.exe' `
        -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -like '*gnu-tools-for-stm32*' } |
        Sort-Object FullName -Descending |
        Select-Object -First 1
    if ($null -eq $tool) {
        throw 'arm-none-eabi-gcc.exe was not found. Pass -ToolchainBin explicitly.'
    }
    $ToolchainBin = $tool.DirectoryName
}

$gcc = Join-Path $ToolchainBin 'arm-none-eabi-gcc.exe'
$objcopy = Join-Path $ToolchainBin 'arm-none-eabi-objcopy.exe'
$size = Join-Path $ToolchainBin 'arm-none-eabi-size.exe'
foreach ($toolPath in @($gcc, $objcopy, $size)) {
    if (-not (Test-Path -LiteralPath $toolPath -PathType Leaf)) {
        throw "Required tool not found: $toolPath"
    }
}

$buildRoot = Join-Path $repoRoot ('.build\' + $Configuration)
$objectRoot = Join-Path $buildRoot 'obj'
New-Item -ItemType Directory -Force -Path $objectRoot | Out-Null

$includeDirectories = @(
    'Core\Inc',
    'Drivers\STM32F4xx_HAL_Driver\Inc',
    'Drivers\STM32F4xx_HAL_Driver\Inc\Legacy',
    'Middlewares\Third_Party\FreeRTOS\Source\include',
    'Middlewares\Third_Party\FreeRTOS\Source\CMSIS_RTOS_V2',
    'Middlewares\Third_Party\FreeRTOS\Source\portable\GCC\ARM_CM4F',
    'Drivers\CMSIS\Device\ST\STM32F4xx\Include',
    'Drivers\CMSIS\Include',
    'FATFS\Target',
    'FATFS\App',
    'Middlewares\Third_Party\FatFs\src'
)
$includeFlags = $includeDirectories | ForEach-Object {
    '-I' + (Join-Path $repoRoot $_)
}

$commonFlags = @(
    '-mcpu=cortex-m4', '-mthumb', '-mfpu=fpv4-sp-d16',
    '-mfloat-abi=hard', '-DUSE_HAL_DRIVER', '-DSTM32F407xx',
    '-ffunction-sections', '-fdata-sections', '-Wall', '-Wextra',
    '--specs=nano.specs'
)
if ($Configuration -eq 'Debug') {
    $configurationFlags = @('-O0', '-g3', '-DDEBUG')
} else {
    $configurationFlags = @('-Os', '-g0')
}

$sourceRoots = @('Core', 'Drivers', 'FATFS', 'Middlewares') |
    ForEach-Object { Join-Path $repoRoot $_ }
$sources = Get-ChildItem -Path $sourceRoots -Recurse -File |
    Where-Object { $_.Extension -in @('.c', '.s') } |
    Sort-Object FullName

$objects = [System.Collections.Generic.List[string]]::new()
Push-Location $repoRoot
try {
    foreach ($source in $sources) {
        $relative = $source.FullName.Substring($repoRoot.Length + 1)
        $objectPath = Join-Path $objectRoot ($relative + '.o')
        New-Item -ItemType Directory -Force -Path (Split-Path $objectPath) |
            Out-Null
        # Project-owned application code is held to warnings-as-errors. ST,
        # FreeRTOS, and FatFs sources retain visible upstream warnings without
        # requiring local edits to generated/vendor code.
        $ownershipFlags = if ($relative.StartsWith('Core\')) {
            @('-Werror')
        } else {
            @()
        }
        if ($source.Extension -eq '.c') {
            $compileArguments = @($commonFlags) + @($configurationFlags) +
                @($ownershipFlags) + @($includeFlags) +
                @('-std=gnu11', '-c', $source.FullName, '-o', $objectPath)
        } else {
            $compileArguments = @($commonFlags) + @($configurationFlags) +
                @($ownershipFlags) + @($includeFlags) +
                @('-x', 'assembler-with-cpp', '-c', $source.FullName,
                    '-o', $objectPath)
        }
        & $gcc @compileArguments
        if ($LASTEXITCODE -ne 0) {
            throw "Compilation failed: $relative"
        }
        $objects.Add($objectPath)
    }

    $elf = Join-Path $buildRoot 'boilerplate_vcu.elf'
    $map = Join-Path $buildRoot 'boilerplate_vcu.map'
    $hex = Join-Path $buildRoot 'boilerplate_vcu.hex'
    $bin = Join-Path $buildRoot 'boilerplate_vcu.bin'
    $linkerScript = Join-Path $repoRoot 'STM32F407VETX_FLASH.ld'
    $linkFlags = @(
        '-mcpu=cortex-m4', '-mthumb', '-mfpu=fpv4-sp-d16',
        '-mfloat-abi=hard', '--specs=nano.specs', '--specs=nosys.specs',
        '-T', $linkerScript, ('-Wl,-Map=' + $map), '-Wl,--cref',
        '-Wl,--gc-sections', '-Wl,--print-memory-usage', '-static'
    )
    $linkArguments = @($linkFlags) + @($objects) +
        @('-Wl,--start-group', '-lc', '-lm', '-Wl,--end-group', '-o', $elf)
    & $gcc @linkArguments
    if ($LASTEXITCODE -ne 0) {
        throw 'Firmware link failed.'
    }
    & $objcopy '-O' 'ihex' $elf $hex
    if ($LASTEXITCODE -ne 0) { throw 'HEX generation failed.' }
    & $objcopy '-O' 'binary' $elf $bin
    if ($LASTEXITCODE -ne 0) { throw 'BIN generation failed.' }
    & $size $elf
    if ($LASTEXITCODE -ne 0) { throw 'Size reporting failed.' }

    $version = (& $gcc --version | Select-Object -First 1)
    $manifest = [ordered]@{
        configuration = $Configuration
        compiler = $version
        source_count = $sources.Count
        elf_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $elf).Hash
        hex_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $hex).Hash
        bin_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $bin).Hash
    }
    $manifest | ConvertTo-Json | Set-Content -Encoding UTF8 `
        -LiteralPath (Join-Path $buildRoot 'build-manifest.json')
    Write-Host "Build completed: $elf"
} finally {
    Pop-Location
}
