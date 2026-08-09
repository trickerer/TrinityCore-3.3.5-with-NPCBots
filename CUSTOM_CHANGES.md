# Custom Changes

This file documents the local changes maintained in this fork.

## Branches

- `feature/useonbot-namedspell`: NPCBot named-spell command and Prayer of Mending support.
- `feature/trial-of-the-champion`: Trial of the Champion fixes, based on the NPCBot branch.

## NPCBot named-spell command

The command resolves a learned positive spell by its localized name and casts it on an active NPCBot.

Example:

```text
.npcbot useonbot namedspell Llane Gebet_der_Besserung
```

The matching HealBot integration is maintained outside this core repository.

## Trial of the Champion

Normal mode was tested through a complete clear with NPCBots.
Heroic mode has not yet been fully repaired or verified.

The local script changes cover:

- Grand Champion encounter progression and combat flow.
- Random selection between Eadric and Paletress.
- Scripted defeat handling for Eadric and Paletress.
- Black Knight phase progression and final completion credit.

### Achievement credit markers

- Paletress: spell `68574`
- Eadric: spell `68575`
- Black Knight: spell `68663`

Eadric and Paletress survive their scripted defeat, so normal creature-death credit does not occur. Their achievement criteria must therefore be updated explicitly.

## Updating from upstream

`origin` points to the upstream NPCBots repository. `fork` points to the personal GitHub fork.

Update the NPCBot base branch first, then merge that updated branch into the Trial of the Champion branch. Git preserves the local commits unless upstream changed the same lines, in which case merge conflicts must be resolved manually.

```text
git fetch origin
git switch feature/useonbot-namedspell
git merge origin/npcbots_3.3.5
git push fork feature/useonbot-namedspell

git switch feature/trial-of-the-champion
git merge feature/useonbot-namedspell
git push fork feature/trial-of-the-champion
```

## Debugging note

Temporary Trial of the Champion spell tracing was removed from the core files. A local backup exists outside the repository at:

```text
/home/katharsis/TrinityCore-untracked-backup-2026-08-09/toc5-spell-debug.patch
```

### Database changes

The matching world-database update is stored in:

`sql/custom/world/2026_08_09_00_world_trial_of_the_champion.sql`

It assigns the standalone lesser-champion script, removes the obsolete vehicle accessories, and restores the usable configuration for all six normal/heroic reward chests.
