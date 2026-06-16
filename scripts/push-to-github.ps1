param(
    [Parameter(Mandatory = $true)]
    [string] $GitHubUser,

    [switch] $Private,

    [switch] $Public
)

$ErrorActionPreference = "Stop"

$repoName = "stereo-to-714-vst3"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

Set-Location $root

function Normalize-GitHubRemoteSlug {
    param([string] $Url)

    $trimmed = $Url.Trim()

    if ($trimmed -match '^https://github\.com/(.+?)(?:\.git)?/?$') {
        return $Matches[1].ToLowerInvariant()
    }

    if ($trimmed -match '^git@github\.com:(.+?)(?:\.git)?$') {
        return $Matches[1].ToLowerInvariant()
    }

    return $trimmed.ToLowerInvariant()
}

# Refresh PATH for gh/git
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" `
          + [System.Environment]::GetEnvironmentVariable("Path", "User")

if ($Private -and $Public) {
    Write-Host "Choose either -Private or -Public, not both." -ForegroundColor Red
    exit 1
}

Write-Host "Checking GitHub login..."
gh auth status | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "You are not logged in. Run this first:" -ForegroundColor Yellow
    Write-Host "  gh auth login" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Then run this script again."
    exit 1
}

$status = git status --porcelain
if ($LASTEXITCODE -ne 0) {
    Write-Host "This folder is not a valid Git repository." -ForegroundColor Red
    exit 1
}

if ($status) {
    Write-Host "There are uncommitted changes that GitHub would not receive." -ForegroundColor Red
    Write-Host "Commit or discard them first, then run this script again:" -ForegroundColor Yellow
    git status --short
    exit 1
}

# Ensure on main branch
git branch -M main 2>$null

$remoteUrl = "https://github.com/$GitHubUser/$repoName.git"
$targetSlug = "$GitHubUser/$repoName".ToLowerInvariant()

$existingOrigin = git remote get-url origin 2>$null
if ($LASTEXITCODE -ne 0) {
    git remote add origin $remoteUrl
} elseif ((Normalize-GitHubRemoteSlug $existingOrigin) -ne $targetSlug) {
    Write-Host "Refusing to overwrite existing origin remote:" -ForegroundColor Red
    Write-Host "  $existingOrigin"
    Write-Host ""
    Write-Host "If this is the right repository, update it yourself and rerun:" -ForegroundColor Yellow
    Write-Host "  git remote set-url origin $remoteUrl" -ForegroundColor Cyan
    exit 1
}

$visibility = if ($Public) { "--public" } else { "--private" }

gh repo view "$GitHubUser/$repoName" --json name --jq .name *> $null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Creating GitHub repo: $GitHubUser/$repoName ($($visibility.TrimStart('-'))) ..."
    gh repo create "$GitHubUser/$repoName" $visibility

    if ($LASTEXITCODE -ne 0) {
        Write-Host "Repository creation failed. Nothing was pushed." -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "Repository already exists: $GitHubUser/$repoName"
}

git push -u origin main
if ($LASTEXITCODE -ne 0) {
    Write-Host "Push failed. See docs/github-setup.md for manual steps." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
Write-Host "Repository: https://github.com/$GitHubUser/$repoName"
Write-Host "Actions:    https://github.com/$GitHubUser/$repoName/actions"
Write-Host ""
Write-Host "Wait for 'Build VST3' to finish, then download artifact:"
Write-Host "  StereoTo714-vst3-macos-universal"
