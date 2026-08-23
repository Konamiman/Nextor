#!/bin/sh
# Generates the categorized releases page, docs/releases, which GitHub Pages
# (configured to publish the docs directory) serves at
# https://konamiman.github.io/Nextor/releases/
#
# The page is generated from the actual releases of the repository and is
# kept in source control: run this script after publishing, editing or
# deleting a release (or after changing config.yaml or MAIN.md), check the
# result, then commit docs/releases and push.
#
# With --fake the page is generated from the releases in fake-releases.json
# instead (a file that contains fabricated releases data for testing), and it is
# written to bin/releases-page rather than to docs/releases, so that a page
# made of fake data can't be committed by accident. This is useful for trying
# changes to the configuration against releases that don't exist yet.
#
# The generator (https://github.com/Konamiman/github-categorized-releases)
# is cloned into bin/categorized-releases on the first run, at the exact
# version tag set in tool_ref below, so that the generated page is the same
# wherever it is regenerated (bin/ is gitignored). To use a newer version,
# change tool_ref and delete bin/categorized-releases so that it is cloned
# again: the script never updates an existing clone. Node.js (20 or later)
# and git are required. Set GITHUB_TOKEN to avoid the anonymous GitHub API
# rate limit (not needed with --fake).
#
# Usage: ./generate.sh [--fake]

set -eu

here="$(cd "$(dirname "$0")" && pwd)"
repo_root="$(cd "$here/../.." && pwd)"
tool_dir="$repo_root/bin/categorized-releases"
tool_repo="https://github.com/Konamiman/github-categorized-releases"
tool_ref="v1.1.0"   # Exact version tag, see the note above
fake_releases="$here/fake-releases.json"

for cmd in node git; do
  command -v "$cmd" >/dev/null 2>&1 || { echo "ERROR: $cmd is required" >&2; exit 1; }
done

case "${1:-}" in
  "")
    source_args="--repo Konamiman/Nextor"
    output_dir="$repo_root/docs/releases"
    ;;
  --fake)
    if [ ! -f "$fake_releases" ]; then
      echo "ERROR: $fake_releases not found (see README.md for its format)" >&2
      exit 1
    fi
    # The repository name comes from the "repository" field of the file
    source_args="--releases-file $fake_releases"
    output_dir="$repo_root/bin/releases-page"
    ;;
  *)
    echo "Usage: $0 [--fake]" >&2
    exit 1
    ;;
esac

if [ ! -f "$tool_dir/src/generate-release-page.js" ]; then
  echo "Cloning $tool_repo ($tool_ref) into $tool_dir..."
  git -c advice.detachedHead=false clone --quiet --depth 1 --branch "$tool_ref" "$tool_repo" "$tool_dir"
  (cd "$tool_dir" && npm ci --ignore-scripts --silent)
fi

# shellcheck disable=SC2086
node "$tool_dir/src/generate-release-page.js" \
  $source_args \
  --config "$here/config.yaml" \
  --output "$output_dir"

echo
echo "Open $output_dir/index.html in a browser to check the page."
if [ "${1:-}" = "" ]; then
  echo "If it looks right, commit docs/releases and push."
fi
