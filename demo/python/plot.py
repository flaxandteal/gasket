import os
import lxml.etree as ET
import re
import sys
import socket
import curses
import svg_histogram
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

histogram_refresh = 3

def run(stdscr):
    stdscr.nodelay(1)
    stdscr.addstr(20, 5, "Curses and Gasket")
    curses.curs_set(0)
    stdscr.refresh()

    exit = False
    counter = 0
    while not exit:
        if counter % histogram_refresh == 0:
            histogram_tree = svg_histogram.init_histogram()
            histogram_svg = ET.tostring(histogram_tree, pretty_print=True)
            train.update_carriage(date_car_id, histogram_svg)

        time.sleep(1)

        exit = (stdscr.getch() == ord('\n'))
        counter += 1

    train.shutdown()

curses.wrapper(run)
