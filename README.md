# Sync

A program for syncing your dotfiles and software across computers.

It works by letting you specify a list of git repos, `make install` commands and scripts.

The idea is that if you, like me, use suckless software and change your configs alot you should easily be able to sync it across your devices.

Simply add all of your suckless applications as make dirs, add your dotfiles repo as a git directory and a rc directory with scripts to copy over your dotfiles.

## Installation

```
git clone https://github.com/depsterr/sync.git
cd sync
sudo make install
```

## Usage

### Setup

For setting up your sync directories run:

```
dsync create [ path to .syncfile ]
```

This will let you specify all directories. 

Then set up your .syncpushrc and .syncpullrc files to copy files if needed (if you have a repo in another location than the file on your computer).

If no syncfile is specified it will write to the current directory as .syncfile.

### Pulling config

Simply run:

```
dsync pull [ path to .syncfile ]
```

If no syncfile is specified it will read from current directory.

### Pushing config

Simply run:

```
dsync push [ path to .syncfile ]
```

If no syncfile is specified it will read from current directory.

### Manually editing .syncfiles

Syncfiles have extremely simple formatting, each row starts with an identifier, `g` for git repos `m` for make folders `r` for rc folders and then an `e` at the end of the file to signify it's end

`g`, `m` and `r` are followed up by a path to the directory, without any whitespace inbetween.
