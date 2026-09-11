$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$output = Join-Path $root '.build\host-tests'
New-Item -ItemType Directory -Force -Path $output | Out-Null

$tests = @(
    @{ Name = 'adc_snapshot'; Sources = @('tests\host\test_adc_snapshot.c',
        'Core\Src\Peripherals\adc_snapshot.c') },
    @{ Name = 'can_rx_policy'; Sources = @('tests\host\test_can_rx_policy.c',
        'Core\Src\Peripherals\can_rx_policy.c') },
    @{ Name = 'can_tx_safety'; Sources = @('tests\host\test_can_tx_safety.c',
        'Core\Src\Peripherals\can_tx_safety.c') },
    @{ Name = 'inverter_fault_status'; Sources = @(
        'tests\host\test_inverter_fault_status.c',
        'Core\Src\Tasks\Task_Helper\motor_control.c') },
    @{ Name = 'pedal_plausibility'; Sources = @(
        'tests\host\test_pedal_plausibility.c',
        'Core\Src\Tasks\Task_Helper\pedal_plausibility.c') },
    @{ Name = 'storage_policy'; Sources = @('tests\host\test_storage_policy.c',
        'Core\Src\Peripherals\storage_policy.c') },
    @{ Name = 'bms_client'; Sources = @('tests\host\test_bms_client.c',
        'Core\Src\Peripherals\bms_client.c') }
)

Push-Location $root
try {
    foreach ($test in $tests) {
        $executable = Join-Path $output ($test.Name + '.exe')
        & gcc '-std=c11' '-Wall' '-Wextra' '-Werror' '-ICore/Inc' `
            @($test.Sources) '-o' $executable
        if ($LASTEXITCODE -ne 0) {
            throw ('Host test compilation failed: ' + $test.Name)
        }
        & $executable
        if ($LASTEXITCODE -ne 0) {
            throw ('Host test failed: ' + $test.Name)
        }
        Write-Host ('PASS ' + $test.Name)
    }
} finally {
    Pop-Location
}
