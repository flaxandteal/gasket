import os
import lxml.etree as ET
from datetime import datetime
import re
import time
import select
import sys
import socket

try :
    from gi.repository import Gasket
except :
    sys.exit("Gasket not found in GIR")

interactive = "-i" in sys.argv

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

command = None
while not command:
    now = datetime.now()
    date_text.text = now.strftime("%Y-%m-%d %H:%M:%S")

    xml = ET.tostring(g, pretty_print=True)
    train.update_carriage(date_car_id, xml)
    #train.redisplay()

    if interactive:
        command, _, _ = select.select([sys.stdin], [], [], 1)
    else:
        time.sleep(1)

    #train.flush()

train.shutdown()
