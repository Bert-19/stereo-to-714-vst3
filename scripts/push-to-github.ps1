param(
    [Parameter(Mandatory = $true)]
    [string] $GitHubUser,

    [switch] $Private
)

$ErrorActionPreference = "Stop"

$repoName = "stereo-to-714-vst3"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

Set-Location $root

# Refresh PATH for gh/git
$env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" `
          + [System.Environment]::GetEnvironmentVariable("Path", "User")

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

# Only publish from the intended default branch. Renaming the current branch here
# could accidentally push feature work to origin/main.
$currentBranch = (git branch --show-current).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($currentBranch)) {
    Write-Host "Unable to determine the current Git branch. Check out main before pushing." -ForegroundColor Red
    exit 1
}

if ($currentBranch -ne "main") {
    Write-Host "Refusing to push branch '$currentBranch' to origin/main." -ForegroundColor Red
    Write-Host "Run 'git switch main' first, then run this script again." -ForegroundColor Yellow
    exit 1
}

$remoteUrl = "https://github.com/$GitHubUser/$repoName.git"

if (-not (git remote get-url origin 2>$null)) {
    git remote add origin $remoteUrl
} else {
    git remote set-url origin $remoteUrl
}

Write-Host "Creating GitHub repo: $GitHubUser/$repoName ..."
$visibility = if ($Private) { "--private" } else { "--public" }

gh repo create "$GitHubUser/$repoName" $visibility --source=. --remote=origin --push 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Repo may already exist, pushing to origin/main ..."
    git push -u origin main
}

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
