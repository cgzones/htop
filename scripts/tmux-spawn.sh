#!/bin/sh

set -eu

SCRIPT=$(readlink -f "$0")
SCRIPTDIR=$(dirname "$SCRIPT")
HTOP="${SCRIPTDIR}/../htop -d5"

exec /usr/bin/tmux new-session \; \
    split-window -v -p 25 "$HTOP" \; \
    split-window -h -p 25 "$HTOP" \; \
    last-pane \; \
    split-window -h -p 33 "$HTOP" \; \
    last-pane \; \
    split-window -h -p 50 "$HTOP" \; \
    \
    select-pane -t 0 \; \
    \
    split-window -v -p 33 "$HTOP" \; \
    split-window -h -p 25 "$HTOP" \; \
    last-pane \; \
    split-window -h -p 33 "$HTOP" \; \
    last-pane \; \
    split-window -h -p 50 "$HTOP" \; \
    \
    select-pane -t 0 \; \
    \
    split-window -v -p 50 "$HTOP" \; \
    split-window -h -p 25 "$HTOP" \; \
    last-pane \; \
    split-window -h -p 33 "$HTOP" \; \
    last-pane \; \
    split-window -h -p 50 "$HTOP" \; \
    \
    select-pane -t 0 \; \
    \
    split-window -h -p 25 "$HTOP" \; \
    last-pane \; \
    split-window -h -p 33 "$HTOP" \; \
    last-pane \; \
    split-window -h -p 50 "$HTOP" \; \
    \
    select-pane -t 0 \; \
    send-keys "$HTOP" C-m\;
