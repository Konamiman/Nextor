# Backport Command

$ARGUMENTS is the URL or number of a merged pull request in the Nextor repository (https://github.com/Konamiman/Nextor/). The goal is to create a draft "backport" pull request that will incorporate the changes from that pull request (targetting Nextor 2) to the Nextor 3 branch, which currently lacks functionality from Nextor 2. Steps:

1. Make sure that the "v3.0" branch is current with the remote. If necessary: stash local changes, pull, and pop changes.
2. Create a "nextor3/<original branch name>" branch (where "original branch name" is the source/head branch from the original PR).
3. Apply the changes from the original pull request, and create one single commit with the following text (only this text, nothing else):

```
<pull request title>

This is a port of PR #<original pr number>
```

Note: do NOT include the changes already present in the working tree in the commit, include exclusively the changes from the original pull request.

4. When creating the commit, handle any merge conflicts as you see fit. If there's a conflict that you can't fix, stop and provide me all the relevant information so that I can decide how to proceed.
5. Create a pull request from the branch and the commit, with the following information:

Title: [Nextor 3] <title of the original pull request>
State: Draft
Target branch: v3.0
Milestone: v3.0.0 alpha 1
Project: Nextor 3 (project number 3), status: In Review
Text:

```
"Backport" of #<original pull request number>

Part of #164

---

<summary of the changes performed, including any merge conflicts handled>
```
