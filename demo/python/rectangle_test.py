import time

try :
    from gi.repository import Gasket
except :
    sys.exit("Gasket not found in GIR")

train = Gasket.Train()
if train is None or train.station_connect() < 0 :
    sys.exit("Gasket could not connect to station (server)")

rect_car_id = train.add_carriage("Rectangle")

if rect_car_id is None:
    sys.exit("Could not create carriage (SVG carrying unit)")

xml_string = """
  <rect x='0' y='0' width='4' height='4'
    style='fill:rgb(255,0,0);fill-opacity:1.'/>
"""

train.update_carriage(rect_car_id, xml_string)
train.redisplay()

train.flush()
time.sleep(1)

train.shutdown()
