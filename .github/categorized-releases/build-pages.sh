#!/bin/sh
# Builds the GitHub Pages site locally, to check how the docs directory (the
# Markdown documentation rendered by Jekyll, plus the categorized releases
# page in docs/releases) will look once published.
#
# The build runs in the very same container GitHub uses for "deploy from a
# branch" sites (ghcr.io/actions/jekyll-build-pages, which runs the
# github-pages gem with its standard plugins and theme), so what comes out is
# what GitHub would publish, with one deliberate difference: the site is built
# with an empty baseurl (GitHub uses /Nextor), so that it can be browsed from
# the root of a local web server.
#
# Requirements:
#   - Docker.
#   - A GitHub token in GITHUB_TOKEN. If the variable is not set and the gh
#     CLI is available, its token ("gh auth token") is used. Any valid token
#     will do, no particular scopes or permissions are needed.
#
# Output: <repo>/bin/pages-site (bin/ is gitignored). To browse it:
#   python3 -m http.server -d bin/pages-site 8000   (then open http://localhost:8000)
# Opening the files directly in the browser doesn't work well: the theme's
# stylesheet and the links between pages are root-relative.
#
# Usage: ./build-pages.sh

set -eu

here="$(cd "$(dirname "$0")" && pwd)"
repo_root="$(cd "$here/../.." && pwd)"
work="$repo_root/bin/pages-build"
out="$repo_root/bin/pages-site"
image="ghcr.io/actions/jekyll-build-pages:v1.0.13"
repo="Konamiman/Nextor"

command -v docker >/dev/null 2>&1 || { echo "ERROR: docker is required" >&2; exit 1; }

if [ -z "${GITHUB_TOKEN:-}" ]; then
  if command -v gh >/dev/null 2>&1; then
    GITHUB_TOKEN="$(gh auth token 2>/dev/null || true)"
  fi
  if [ -z "${GITHUB_TOKEN:-}" ]; then
    echo "ERROR: a GitHub token is required (set GITHUB_TOKEN, or log in with 'gh auth login')" >&2
    exit 1
  fi
fi

if [ ! -f "$repo_root/docs/releases/index.html" ]; then
  echo "WARNING: docs/releases/index.html not found, the site will have no releases page (run generate.sh)" >&2
fi

# Assemble the Jekyll source: a copy of docs/ with the baseurl override
# appended to the site configuration.
rm -rf "$work"
mkdir -p "$work/src" "$work/out"
cp -R "$repo_root/docs/." "$work/src/"
printf '\n# Added by build-pages.sh for local browsing (GitHub uses /Nextor)\nbaseurl: ""\n' >> "$work/src/_config.yml"

# Same environment the Actions runner gives the container. It runs as the
# current user so that the output is owned by us and not by root.
docker run --rm \
  --user "$(id -u):$(id -g)" \
  -e HOME=/tmp \
  -v "$work":/github/workspace \
  -e GITHUB_WORKSPACE=/github/workspace \
  -e GITHUB_API_URL=https://api.github.com \
  -e GITHUB_REPOSITORY="$repo" \
  -e INPUT_TOKEN="$GITHUB_TOKEN" \
  -e INPUT_SOURCE=src \
  -e INPUT_DESTINATION=out \
  -e INPUT_FUTURE=false \
  -e INPUT_VERBOSE=false \
  -e INPUT_BUILD_REVISION=local \
  "$image"

rm -rf "$out"
mv "$work/out" "$out"
rm -rf "$work"

echo
echo "Site built in $out. To browse it:"
echo "  python3 -m http.server -d $out 8000"
echo "then open http://localhost:8000 (the releases page is at /releases/)."
