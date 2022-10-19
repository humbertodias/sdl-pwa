#!/usr/bin/env sh

set -e

mkdir -p web
cp -R res web.cpp Makefile index.html web
cd web
make web

GIT_REPOSITORY=`git config --get remote.origin.url`

git init
git add -A
git commit -m 'deploy'
git push -f $GIT_REPOSITORY `git rev-parse HEAD`:gh-pages

cd -