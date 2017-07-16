# Hacklily Standalone

This is a standalone desktop version of Hacklily.

This repo neither contains the UI nor the Lilypond backend. Instead, it is a wrapper
that downloads both the JavaScript UI and Lilypond backend and ties them together.

It uses Qt and QWebEngine (Chromium).

It can automatically update the UI and the backend, but does not have an update mechanism
for the wrapper itself.

## Hacking

Download Qt 5.9 and open up hacklily-standalone.pro and open up Qt Creator. Or just run

```
cd hacklily-standalone
mkdir build
cd build
qmake ..
make
```
