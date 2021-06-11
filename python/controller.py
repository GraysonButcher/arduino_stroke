#!/bin/bash/python3
import os
import time
import platform
from comporthandler import cph
from datafile import df

refresh_com_port_timeout = 2  # refresh COM port information every THIS second(s)
new_data_in_file_timeout = 5  # time between looking for new data in the data file


class PilotError(Exception):
    # Used if there's an error importing stuff
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


if platform.system().lower() != "windows":
    raise PilotError('This code is designed to run on Windows.')


def print_to_screen():
    #  Keeps the screen cleared of clutter and lets the user know which ports see an Arduino

    os.system('cls')
    msg = "Known ports with Arduinos:\n"
    for port in cph.known_arduinos.keys():
        msg += port + "\n"
    msg += df.error_msg
    print(msg.rstrip('\n'))


def main():
    last_refresh_time = 0
    last_data_file_refresh = 0

    while True:
        if time.time() - last_refresh_time > refresh_com_port_timeout:
            #  Every so often, let's refresh our knowledge of what's connected where

            cph.scan_ports()
            print_to_screen()
            last_refresh_time = time.time()

        if time.time() - last_data_file_refresh > new_data_in_file_timeout:
            #  Checks if the data file was updated by the researcher

            df.get_data_from_file()
            last_data_file_refresh = time.time()

        cph.read_and_log_data()


if __name__ == "__main__":
    #  This is python-ese for "this module is the primary entry point
    #  This will only be true if you run "python controller.py" from the command line
    #  If other code imports this module, this will not be true.

    main()
