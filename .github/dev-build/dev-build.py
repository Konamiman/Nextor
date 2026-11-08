#!/usr/bin/env python3
"""
Decides what the dev build workflow (.github/workflows/dev-build.yaml) has to
do, and generates the release notes and the list of assets for the dev build
release.

"Unreleased changes" are the first-parent commits of the base branch (merged
pull requests, or commits pushed directly) that touch files affecting the
dev build assets AND are not included in the newest release that ships the
affected assets. Nextor 3 releases each component separately, and a release
tag contains everything merged before it, so plain "is it reachable from any
release tag" doesn't work: a release of just mknexrom would hide all the
pending kernel changes. Instead, every changed file is mapped to the
components it affects (see components_for), and the commit counts as
released only when it's contained in the newest release of each of those
components. A component's newest release is found by asset names (the same
way the categorized releases page does it, see
.github/categorized-releases/config.yaml), since the tag names are not
consistent.

The resulting action (written to $GITHUB_OUTPUT when running in GitHub
Actions, printed otherwise):

- build:   there are unreleased changes: build the commit and publish it as
           the dev build release.
- delete:  there are no unreleased changes (everything was included in
           official releases) but a dev build release exists: delete it,
           together with its tag.
- none:    no unreleased changes and no dev build release: nothing to do.

Requirements: git (with the full history and the release tags fetched) and
the GitHub CLI (gh) authenticated with GH_TOKEN. Run with --help for the
arguments; running it locally is harmless, it doesn't change anything.
"""

import argparse
import json
import os
import re
import subprocess
import sys

# Releases never considered: the dev build itself (added at runtime) and the
# Nextor 2 releases, which live in their own branch.
EXCLUDED_TAGS = re.compile(r"^v2\.")

# Components, identified by the names of the release assets that ship them.
COMPONENTS = {
    "kernel": re.compile(r"^Nextor-.*\.base(\..*)?\.dat$", re.I),
    "roms": re.compile(r"^Nextor-.*\.Standalone(ASCII8|ASCII16).*\.ROM$", re.I),
    "ramdrv": re.compile(r"^ram-driver-example.*\.drv$", re.I),
    "disk": re.compile(r"^nextor.*\.dsk$", re.I),
}

# The components that are always released together for a new Nextor version.
CORE = {"kernel", "roms", "disk"}


def components_for(path):
    """
    The components affected by a change in the given file, by path. This is
    a deliberate approximation, it doesn't track every build dependency
    (NEXTOR.SYS and some tools include kernel files, for example): a change
    is attributed to the components that its directory produces, and the
    files that are shared by the whole build go to the core components.
    The example RAM driver is attributed only the changes to its own source
    file, since it's rarely re-released.

    Files that don't affect the assets (documentation, workflows,
    templates...) return an empty set.
    """
    if path.lower().endswith(".md"):
        return set()
    if path.startswith("source/kernel/"):
        return {"kernel", "roms"}
    if path == "source/drivers/ram-driver-example.asm":
        return {"ramdrv"}
    if path.startswith(("source/drivers/", "buildtools/")):
        return {"roms"}
    if path.startswith(("source/tools/", "source/commandcom/", "source/nextor_sys/", "source/msxdos1/")):
        return {"disk"}
    if path.startswith("sdk/templates/"):
        return set()
    if path.startswith(("source/", "sdk/")) or path == "docker/Dockerfile":
        return set(CORE)
    return set()


def run(*args, check=True, env=None):
    result = subprocess.run(args, capture_output=True, text=True, env=env)
    if check and result.returncode != 0:
        sys.exit(f"Command failed: {' '.join(args)}\n{result.stderr}")
    return result


def git(*args):
    return run("git", *args).stdout.strip()


def gh_api(path, jq=None, paginate=False):
    """GET from the GitHub API, returns None on 404."""
    args = ["gh", "api", path]
    if paginate:
        args.append("--paginate")
    if jq:
        args += ["--jq", jq]
    result = run(*args, check=False)
    if result.returncode != 0:
        if "Not Found" in result.stderr or "HTTP 404" in result.stderr:
            return None
        sys.exit(f"GitHub API call failed: {path}\n{result.stderr}")
    return result.stdout


def is_ancestor(commit, of):
    return run("git", "merge-base", "--is-ancestor", commit, of, check=False).returncode == 0


def changed_files(commit):
    """Files changed by a first-parent commit (for a merge: by the whole merged branch)."""
    parents = git("rev-list", "--parents", "-n", "1", commit).split()[1:]
    if not parents:
        return []
    return git("diff", "--name-only", parents[0], commit).splitlines()


def components_of_files(files):
    result = set()
    for f in files:
        result |= components_for(f)
    return result


def get_baselines(repo, dev_tag):
    """For each component: (tag, commit) of its newest release."""
    out = gh_api(
        f"repos/{repo}/releases?per_page=100",
        jq=".[] | {tag: .tag_name, draft: .draft, published: .published_at, assets: [.assets[].name]}",
        paginate=True,
    )
    releases = [json.loads(line) for line in out.splitlines() if line.strip()]
    releases = [
        r for r in releases
        if not r["draft"] and r["published"] and r["tag"] != dev_tag and not EXCLUDED_TAGS.search(r["tag"])
    ]
    releases.sort(key=lambda r: r["published"], reverse=True)

    baselines = {}
    for name, regex in COMPONENTS.items():
        release = next((r for r in releases if any(regex.search(a) for a in r["assets"])), None)
        if release is None:
            print(f"::warning::No release found for component '{name}', its changes won't be listed")
            continue
        commit = run("git", "rev-parse", "--verify", "--quiet", f"{release['tag']}^{{commit}}", check=False).stdout.strip()
        if not commit:
            sys.exit(f"The tag {release['tag']} (newest release of component '{name}') is not available locally: fetch the tags")
        baselines[name] = (release["tag"], commit)
    return baselines


def get_unreleased(repo, base, target, baselines):
    """
    The unreleased changes contained in the target commit, oldest first:
    a list of {"pr": {...} or None, "commits": [sha...], "subject": str}.
    """
    candidates = []
    seen = set()
    for _, commit in baselines.values():
        for sha in git("rev-list", "--first-parent", target, f"^{commit}").splitlines():
            if sha not in seen:
                seen.add(sha)
                candidates.append(sha)

    # Oldest first, in first-parent order
    order = {sha: i for i, sha in enumerate(git("rev-list", "--first-parent", target).splitlines())}
    candidates.sort(key=lambda sha: order.get(sha, 0), reverse=True)

    changes = []
    by_pr = {}
    for sha in candidates:
        affected = components_of_files(changed_files(sha))
        unreleased = [c for c in affected if c in baselines and not is_ancestor(sha, baselines[c][1])]
        if not unreleased:
            continue

        out = gh_api(
            f"repos/{repo}/commits/{sha}/pulls",
            jq=f'[.[] | select(.merged_at != null and .base.ref == "{base}") | '
               "{number, title, url: .html_url, merge_commit_sha}]",
        )
        prs = json.loads(out) if out else []
        pr = next((p for p in prs if p["merge_commit_sha"] == sha), prs[0] if prs else None)
        if pr is None:
            changes.append({"pr": None, "commits": [sha], "subject": git("log", "-1", "--format=%s", sha)})
        elif pr["number"] in by_pr:
            by_pr[pr["number"]]["commits"].append(sha)   # Rebase-merged PR
        else:
            change = {"pr": pr, "commits": [sha], "subject": pr["title"]}
            by_pr[pr["number"]] = change
            changes.append(change)
    return changes


def asset_map(version, date):
    """(path of the built file, name of the published asset) for each dev build asset."""
    suffix = f"dev-{date}"
    v = f"Nextor-{version}"
    vn = f"Nextor-{version}-{suffix}"
    return [
        (f"bin/kernel-base/{v}.base.dat", f"{vn}.base.dat"),
        (f"bin/kernel-base/{v}.base.NO_UNDOC.dat", f"{vn}.base.NO_UNDOC.dat"),
        (f"bin/drivers/{v}.StandaloneASCII8.ROM", f"{vn}.StandaloneASCII8.ROM"),
        (f"bin/drivers/{v}.StandaloneASCII8.NO_UNDOC.ROM", f"{vn}.StandaloneASCII8.NO_UNDOC.ROM"),
        (f"bin/drivers/{v}.StandaloneASCII16.ROM", f"{vn}.StandaloneASCII16.ROM"),
        (f"bin/drivers/{v}.StandaloneASCII16.NO_UNDOC.ROM", f"{vn}.StandaloneASCII16.NO_UNDOC.ROM"),
        ("bin/drivers/ram-driver-example.drv", f"ram-driver-example-{suffix}.drv"),
        ("bin/tools/nextor.dsk", f"nextor-{suffix}.dsk"),
        ("bin/tools/tools.zip", f"tools-{suffix}.zip"),
    ]


def make_notes(repo, target, date_iso, build_id, version, date, changes):
    short = target[:7]
    names = [name for _, name in asset_map(version, date)]
    lines = [
        "> [!WARNING]",
        "> **This is not an official release.** It's a build of the latest development code of Nextor,"
        " published so that new features and fixes can be tested before they make it into an official release."
        " It may contain bugs: please report any problem you find in"
        f" [the issues section](https://github.com/{repo}/issues).",
        "",
        f"Built from commit [{short}](https://github.com/{repo}/commit/{target}) ({date_iso})."
        f" The kernel shows `Dev build {build_id}` under the version number when booting.",
        "",
        "This release is replaced when a new dev build is published, and this list isn't updated in between,"
        " so some of the changes below might have been included in an official release already. For the official"
        " releases see [the categorized releases page](https://konamiman.github.io/Nextor/releases/).",
        "",
        "## Assets",
        "",
        f"- `{names[0]}`, `{names[1]}`: the kernel base files, to be combined with a driver using mknexrom.",
        f"- `{names[2]}`, `{names[3]}`, `{names[4]}`, `{names[5]}`: the standalone kernel ROMs"
        " (with a dummy driver that handles no devices).",
        f"- `{names[6]}`: the example RAM driver.",
        f"- `{names[7]}`, `{names[8]}`: the tools disk image and ZIP file, with NEXTOR.SYS, COMMAND3.COM,"
        " the MSX-DOS 1 system files, all the command line tools and the COMMAND3.COM help files.",
        "",
        "Files with `NO_UNDOC` in their name don't use undocumented Z80 instructions, so they work on Z180 based machines.",
        "",
        "## Changes not included in any official release yet",
        "",
    ]
    prs = [c for c in changes if c["pr"]]
    commits = [c for c in changes if not c["pr"]]
    for c in prs:
        lines.append(f"- #{c['pr']['number']} {c['subject']}")
    if commits:
        if prs:
            lines += ["", "Other changes (committed without a pull request):", ""]
        for c in commits:
            sha = c["commits"][0]
            lines.append(f"- [{sha[:7]}](https://github.com/{repo}/commit/{sha}) {c['subject']}")
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--repo", required=True, help="owner/name of the GitHub repository")
    parser.add_argument("--base", default="master", help="branch the dev build is built from (default: master)")
    parser.add_argument("--head", default="HEAD", help="commit to build (default: HEAD)")
    parser.add_argument("--tag", default="dev-build", help="tag of the dev build release (default: dev-build)")
    parser.add_argument("--out-dir", required=True, help="directory for notes.md and assets.tsv")
    args = parser.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)
    target = git("rev-parse", "--verify", f"{args.head}^{{commit}}")
    baselines = get_baselines(args.repo, args.tag)
    for name, (tag, commit) in baselines.items():
        print(f"Newest release of {name}: {tag} ({commit[:7]})")

    changes = get_unreleased(args.repo, args.base, target, baselines)

    if changes:
        # The commit date (in UTC), not the build date: rebuilding the same
        # commit gives the same names.
        date_iso = run("git", "show", "-s", "--format=%cd", "--date=format-local:%Y-%m-%d", target,
                       env={**os.environ, "TZ": "UTC"}).stdout.strip()
        date = date_iso.replace("-", "")
        version = git("show", f"{target}:sdk/nextor-kernel-version.txt")
        build_id = f"{date_iso} ({target[:7]})"
        outputs = {
            "action": "build",
            "reason": f"{len(changes)} unreleased change(s)",
            "target": target,
            "version": version,
            "build_id": build_id,
            "title": f"Dev build {build_id}",
        }
        with open(os.path.join(args.out_dir, "notes.md"), "w") as f:
            f.write(make_notes(args.repo, target, date_iso, build_id, version, date, changes))
        with open(os.path.join(args.out_dir, "assets.tsv"), "w") as f:
            for src, name in asset_map(version, date):
                f.write(f"{src}\t{name}\n")
    else:
        exists = gh_api(f"repos/{args.repo}/releases/tags/{args.tag}", jq=".id") is not None
        outputs = {
            "action": "delete" if exists else "none",
            "reason": "there are no unreleased changes",
        }

    for key, value in outputs.items():
        print(f"{key}={value}")
    if "GITHUB_OUTPUT" in os.environ:
        with open(os.environ["GITHUB_OUTPUT"], "a") as f:
            for key, value in outputs.items():
                f.write(f"{key}={value}\n")


if __name__ == "__main__":
    main()
