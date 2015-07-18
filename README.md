# Gasket
## Client Library

> The crema in your espresso

## Summary

The Gasket Project is a set of patches and a standalone library to provide an SVG overlay inside
libvte-based terminal applications, such as Gnome Terminal, accessible within the PTY as a
Unix domain socket. This allows command-line applications to augment their stdout/curses interfaces
with vector graphics.

This component is the library that may be used by clients to simplify the process of interacting
with Gasket, as well as providing certain server-side routines. Note that it is not compulsory for
clients to use this library. The Gasket protocol is extremely simple, SVG with certain markers, and
can be produced by a shell-script and piped to the terminal's Gasket socket with little difficulty.

## Dependencies

* librsvg2
* uuid
* libgirepository-1.0
* libxml2
* 

## Presentation

A brief introduction (presented to Belfast LUG) is available
at [slides.com/flaxandteal/deck](http://slides.com/flaxandteal/deck "Slide.com Gasket Presentation")
