# Gasket
## Client Library

> The crema in your espresso

## Summary

The Gasket Project is a set of patches and a standalone library to provide an SVG overlay inside
libvte-based terminal applications, such as Gnome Terminal, accessible within the PTY as a
Unix domain socket. This allows command-line applications to augment their stdout/curses interfaces
with vector graphics.

This component is the library that may be used by clients to simplify the process of interacting
with Gasket, as well as providing certain server-side routines. While it must be available to the
server, that is the Gnome Terminal / VTE, Note that it is not compulsory for clients to use this
library. The Gasket protocol is extremely simple, SVG with certain markers, and can be produced
by a shell-script and piped to the terminal's Gasket socket with little difficulty.

## Dependencies

* librsvg2
* uuid
* libgirepository-1.0
* libxml2

## Presentation

A brief introduction (presented to Belfast LUG) is available
at [slides.com/flaxandteal/deck](http://slides.com/flaxandteal/deck "Slide.com Gasket Presentation")

## Installation

This library follows a standard automake approach.
```
./autogen.sh
./configure --path=TARGETROOT
make
make install
```

The process of compiling the VTE component is identical to the normal VTE install, as documented
in that tree. At time of writing, only two additional Gasket specific files are included, and
the upstream scripts have been adjusted to pull them into the build.

It is strongly recommended, at this early alpha stage, that you target a local directory or
virtual environment, including the libraries only when necessary.

Once the Gasket library and updated VTE are available, ensure the modified VTE library will be
loaded ahead of the (upstream) system library, then launch Gnome Terminal. If you do not have
a Gnome installation, or wish to test Gasket in semi-isolation, you should be able to run the
`vte-2.91` binary instead, to bring up a relatively minimalist terminal.

When you receive a prompt, you should see the line:
    `Setting up Gasket for child pty: UUID = xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`
with a UUID indicating the Gasket socket. You can further confirm Gasket's presence by running
`env | grep GASKET`. This should show
```
GASKET_ID=[UUID]
GASKET_SOCKET=/tmp/gasket-[TERMPID]/gasket_track_[UUID].sock
```
with the same UUID.

In the `demo` subdirectory are a couple of test programs. You may try `gasket-square` for the
most basic functionality. This is implemented both in Python and C, and shows a red rectangle
in the top left corner of the terminal. A more advanced example is `gasket_system_monitor.py`
which shows a `top`-like ncurses tool with an embedded graph. Note that this has
additional dependency on the Python packages `lxml`, `curses` and `matplotlib`.
