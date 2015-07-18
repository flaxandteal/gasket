import os
import argparse
import lxml.etree as ET
import re
import sys
import socket
import svg_histogram
import time
import curses

try :
    from gi.repository import Gasket
except :
    sys.exit("Gasket not found in GIR")


def run(stdscr, train, histogram_car_id, histogram_refresh=3):
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
            train.update_carriage(histogram_car_id, histogram_svg)

        time.sleep(1)

        exit = (stdscr.getch() == ord('\n'))
        counter += 1

    train.shutdown()


def main():
    train = Gasket.Train()

    if train is None or train.station_connect() < 0 :
        sys.exit("Gasket could not connect to station (server)")

    histogram_car_id = train.add_carriage("Histogram")

    if histogram_car_id is None:
        sys.exit("Could not create carriage (SVG carrying unit)")

    curses.wrapper(lambda s: run(s, train, histogram_car_id))

if __name__ == "__main__":
    main()
