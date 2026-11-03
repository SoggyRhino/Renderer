set windows-shell := ["C:/Program Files/Git/bin/bash.exe", "-cu"]

# Default recipe
default:
    @just --list

# Clones glTF-Sample-Assets into a temp dir and copies glTF assets to models/<Model_Name>/
sync-models:
    #!powershell -Command
    $ErrorActionPreference = "Stop"
    $RepoUrl = "https://github.com/KhronosGroup/glTF-Sample-Assets.git"
    $TempDir = Join-Path ([System.IO.Path]::GetTempPath()) ([System.Guid]::NewGuid().ToString())

    try {
        Write-Host "==> Cloning sample assets repository (sparse clone)..."
        git clone --depth 1 --filter=blob:none --sparse $RepoUrl $TempDir
        git -C $TempDir sparse-checkout set Models

        $SrcModelsDir = Join-Path $TempDir "Models"
        $DestDir = "models"

        Write-Host "==> Processing models..."
        Get-ChildItem -Path $SrcModelsDir -Directory | ForEach-Object {
            $srcGltf = Join-Path $_.FullName "glTF"
            if (Test-Path $srcGltf) {
                $target = Join-Path $DestDir $_.Name
                New-Item -ItemType Directory -Force -Path $target | Out-Null
                Copy-Item -Path "$srcGltf\*" -Destination $target -Recurse -Force
                Write-Host "Copied: $($_.Name)"
            }
        }
        Write-Host "==> Finished importing models into $DestDir/"
    }
    finally {
        if (Test-Path $TempDir) {
            Remove-Item -Recurse -Force $TempDir
        }
    }