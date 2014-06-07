#!/usr/bin/env python
# Thanks also to
#   david.huard@gmail.com for svg_histogram.py
# -*- noplot -*-
import time
from StringIO import StringIO
import lxml.etree as ET
import matplotlib
import matplotlib.pyplot as plt
import math

#http://www.raspberrypi.org/forums/viewtopic.php?f=32&t=22180
#http://matplotlib.org/examples/user_interfaces/svg_histogram.html
#http://stackoverflow.com/questions/7187504/set-data-and-autoscale-view-matplotlib

import os

# Return RAM information (unit=kb) in a list
# Index 0: total RAM
# Index 1: used RAM
# Index 2: free RAM
def getRAMinfo():
    p = os.popen('free')
    i = 0
    while 1:
        i = i + 1
        line = p.readline()
        if i==2:
            return(line.split()[1:4])

# Return % of CPU used by user as a character string
def getCPUuse():
    return(str(os.popen("top -n1 | awk '/Cpu\(s\):/ {print $2}'").readline().strip(\
)))

# Return information about disk space as a list (unit included)
# Index 0: total disk space
# Index 1: used disk space
# Index 2: remaining disk space
# Index 3: percentage of disk used
def getDiskSpace():
    p = os.popen("df -h /")
    i = 0
    while 1:
        i = i +1
        line = p.readline()
        if i==2:
            return(line.split()[1:5])

def get_stats():
    stats = {}
    # CPU informatiom
    stats["CPU_usage"] = getCPUuse()

    # RAM information
    # Output is in kb, here I convert it in Mb for readability
    RAM_stats = getRAMinfo()
    stats["RAM_total"] = round(int(RAM_stats[0]) / 1000,1)
    stats["RAM_used"] = round(int(RAM_stats[1]) / 1000,1)
    stats["RAM_free"] = round(int(RAM_stats[2]) / 1000,1)

    # Disk information
    DISK_stats = getDiskSpace()
    stats["DISK_total"] = DISK_stats[0]
    stats["DISK_free"] = DISK_stats[1]
    stats["DISK_perc"] = DISK_stats[3]

    return stats

# turn interactive mode on for dynamic updates.  If you aren't in
# interactive mode, you'll need to use a GUI event handler/timer.
#ion()

class SystemMonitor:
    def init_plot(self):
        fig, ax = plt.subplots()
        ax2 = ax.twinx()
        self.axes = [ax, ax2, ax2.twinx()]
        fig.subplots_adjust(right=0.75)

        self.axes[1].yaxis.set_ticks_position('right')
        self.axes[-1].spines['right'].set_position(('axes', 1.2))
        self.axes[-1].set_frame_on(True)

        self.memory = {}
        self.disk = {}
        self.cpu = {}
        stats = get_stats()
        self.update_stats()
        matplotlib.rc('font', family='monospace', weight='normal', size='16')

        self.lines = [{
            "label": "RAM     ______",
            "data": self.memory,
            "colour": 'grey',
            "style": '-',
            "limits": (0.0, float(stats["RAM_total"])),
            }, {
            "label": "Disk %    __ __",
            "data": self.disk,
            "colour": 'grey',
            "style": '--',
            "limits": (0.0, 100.0),
            }, {
            "label": "CPU %     __ . __ .",
            "data": self.cpu,
            "colour": 'grey',
            "style": '-.',
            "limits": (0.0, 100.0),
            }]

        for ax, line in zip(self.axes, self.lines):
            ax.patch.set_visible(False)
            data = zip(*line["data"].items())
            line['line'], = ax.plot(data[0], data[1], color=line["colour"], linestyle=line["style"])
            line['axis'] = ax
            if 'limits' in line:
                ax.set_ylim(line['limits'])
            ax.set_ylabel(line["label"], color=line["colour"])
            ax.tick_params(axis='y', colors=line["colour"])
        ax = self.axes[0]
        ax.set_xlabel("Time", color='white')
        ax.tick_params(axis='x', colors='white')

    def update_stats(self):
        stats = get_stats()
        now = time.time()

        self.memory[now] = stats["RAM_used"]
        self.disk[now] = stats["DISK_perc"][:-1]
        self.cpu[now] = stats["CPU_usage"]

    def update_plot(self):
        for line in self.lines:
            data = sorted(line["data"].items())
            fx = data[-1][0]
            data = filter(lambda x: (x[0] > fx - 20), data)
            data = zip(*data)

            line['line'].set_data(data[0], data[1])
            if 'limits' not in line:
                line['axis'].relim()
                line['axis'].autoscale_view(True, False, True)
            line['axis'].set_xlim(data[0][-1] - 20, data[0][-1] + 2)

        plt.draw()

        svg = StringIO()
        plt.savefig(svg, format="svg", transparent=True)
        tree, xmlid = ET.XMLID(svg.getvalue())

        parent = ET.Element("g", transform="scale(0.08, 0.03) translate(150.0, 50.0)")
        parent.append(tree)

        return ET.ElementTree(parent)
