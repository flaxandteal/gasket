# Written 2013- by Phil Weir
#
# To the extent possible under law, the author have dedicated all
# copyright and related and neighboring rights to this software to
# the public domain worldwide. This software is distributed without
# any warranty.
#
# CC0 Public Domain Dedication:
# http://creativecommons.org/publicdomain/zero/1.0/

import os
import lxml.etree as ET
import re
import sys
import socket
import curses
import system_monitor
import time

try :
    from gi.repository import Gasket
except :
    sys.exit("Gasket not found in GIR")

train = Gasket.Train()
if train is None or train.station_connect() < 0 :
    sys.exit("Gasket could not connect to station (server)")

date_car_id = train.add_carriage("Date")

if date_car_id is None:
    sys.exit("Could not create carriage (SVG carrying unit)")

g = ET.Element('g')

date_properties = {
        'fill' : '#DDFFDD',
        'font-size': '3',
}
hostname_properties = {
        'fill' : '#CCFFFF',
        'font-size': '2',
}

text_properties = {
        'font-family': 'Sans',
        'fill-opacity': '0.3',
        'font-weight': 'bold',
        'transform': 'scale(2, 1)',
}
date_properties.update(text_properties)
hostname_properties.update(text_properties)
date_text = ET.SubElement(
        g,
        'text',
        x='0',
        y='2',
        **date_properties)
hostname_text = ET.SubElement(
        g,
        'text',
        x='0',
        y='4',
        **hostname_properties)
hostname_text.text = socket.gethostname()

histogram_refresh = 1
monitor = system_monitor.SystemMonitor()
monitor.init_plot()
#monitor.update_stats()
#monitor_tree = monitor.update_plot()
#quit()

def run(stdscr):
    stdscr.nodelay(1)
    stdscr.addstr(19, 5, "[-- Curses and Gasket --]")
    stdscr.addstr(20, 5, "CPU usage: ")
    stdscr.addstr(20, 25, "RAM avail.: ")
    stdscr.addstr(21, 25, "RAM used: ")
    stdscr.addstr(22, 25, "RAM free: ")
    stdscr.addstr(20, 45, "Disk total: ")
    stdscr.addstr(21, 45, "Disk used: ")
    stdscr.addstr(22, 45, "Disk free: ")

    curses.curs_set(0)
    stdscr.move(19, 5)
    stdscr.refresh()

    exit = False
    counter = 0
    while not exit:
        monitor.update_stats()
        monitor_tree = monitor.update_plot()
        monitor_svg = ET.tostring(monitor_tree, pretty_print=True)
        train.update_carriage(date_car_id, monitor_svg)

        stats = system_monitor.get_stats()
        stdscr.addstr(20, 16, str(stats["CPU_usage"]).ljust(7))
        stdscr.addstr(20, 37, str(stats["RAM_total"]).ljust(7))
        stdscr.addstr(21, 37, str(stats["RAM_used"]).ljust(7))
        stdscr.addstr(22, 37, str(stats["RAM_free"]).ljust(7))
        stdscr.addstr(20, 58, str(stats["DISK_total"]).ljust(7))
        stdscr.addstr(21, 58, str(stats["DISK_perc"]).ljust(7))
        stdscr.addstr(22, 58, str(stats["DISK_free"]).ljust(7))
        stdscr.move(19, 5)

        time.sleep(histogram_refresh)

        exit = (stdscr.getch() == ord('\n'))
        counter += 1

    train.shutdown()

curses.wrapper(run)
