import os
import re
import time
from logging import Log
from logging import PilotError
from datafile import df
from stimulator import stim

try:
    import serial
    import serial.tools.list_ports
except ModuleNotFoundError:
    raise PilotError('Pyserial is not present on system.\
          Install using "pip install serial" or Anaconda Navigator if using Anaconda.')

class ComPortHandler:
    def __init__(self):
        # known_arduinos follows the structure:
        # {
        #     "COM<x>": {
        #         "hwid": hardware ID of the com port,
        #         "timer": timestamp of the last heartbeat
        #         "sobj": com port's object
        #         "log": log object
        #         "rfid": rat's RFID in the current session
        #     }
        # }

        # Communications Protocol:
        # All communications happens with a start character of "<" and end character of ">"
        #
        # Requests for data from the file are of the form <request#>
        #     where "#" is the RFID of the rat
        #
        # Requests to write the data to the file is in the form of <write#1,#2,#3>
        #     where "#1" is the RFID of the rat,
        #     "#2" is the force threshold
        #     "#3" is the handle position

        self.known_arduinos = {}
        self.heartbeat_timer = 4
        self.com_port_error_count = 0
        self.max_com_port_errors_before_rescan = 50
        self.request_re_obj = re.compile('<request(\d{17}?)>')  # Look for "request" followed by 17 digits encased between "<" and ">"
        self.write_re_obj = re.compile("<write(\d{17},\d{1,5}\.?\d{0,5},\d{1,5}\.?\d{0,5})>") # Look for "write" followed by 17 digit int, a 5 digit float and a 5 digit float all comma separated

    def scan_ports(self):
        #  Scans all known COM ports bound to anything
        #  Filters port list looking for arduinos
        #  Able to handle plugging and unplugging of new and old arduinos with careless abandon

        port_information = serial.tools.list_ports.comports()  # Gets list of all COM ports
        known_port_hwids = {}
        ports_to_remove = []
        self.com_port_error_count = 0

        for port, desc, hwid in sorted(port_information):
            known_port_hwids[port] = hwid
            if (port not in self.known_arduinos.keys()) and ("arduino" in desc.lower()):
                #  Found a new Arduino!

                self.known_arduinos[port] = {}
                self.known_arduinos[port]["sobj"] = \
                    serial.Serial(port=port, baudrate=9600, timeout=1)
                self.known_arduinos[port]["hwid"] = hwid
                self.known_arduinos[port]["log"] = Log(port)

        for port in self.known_arduinos.keys():
            if (port not in known_port_hwids.keys()) or \
                    (self.known_arduinos[port]["hwid"] not in known_port_hwids.values()):
                #  Found an old arduino port that's no longer in use.

                ports_to_remove.append(port)

        if ports_to_remove:
            for port in ports_to_remove:
                self.known_arduinos[port]["sobj"].close()
                del self.known_arduinos[port]

    def read_and_log_data(self):
        #  Reads data from serial port
        #  Converts from bit field to ASCII for logging in human-readable form
        #  If the micro is requesting data for a new RFID, send it
        #  If the micro is requesting learned data for an RFID be written, writes data to the data file
        #  Writes all activity to log file

        for port in self.known_arduinos.keys():
            try:
                data = self.known_arduinos[port]["sobj"].readline().rstrip().decode('ascii')

            except serial.serialutil.SerialException:
                if self.com_port_error_count == 0:
                    self.known_arduinos[port]["log"].log("Failed to get information from port {}. Did it come unplugged?".format(port))

                self.com_port_error_count += 1

                return

            except SyntaxError:
                data = "Unable to understand data from serial port, check baud configuration."
            
            except UnicodeDecodeError:
                self.known_arduinos[port]["log"].log("WARNING: Decoding problem detected. The USB connection might be suspect.")

            if data:
                self.known_arduinos[port]["timer"] = time.time()

                if self.request_re_obj.search(data):
                    self.handle_data_request(data, port)

                elif self.write_re_obj.search(data):
                    self.handle_write_request(data, port)

                elif data == "<stimulate>":
                    self.handle_stim_request(port)

                elif data == "h":
                    self.reset_heartbeat_timer(port)

                else:
                    # Just a standard log packet and not a data request or write
                    self.known_arduinos[port]["log"].log(data)

            elif (("timer" in self.known_arduinos[port]) and
                    (time.time() - self.known_arduinos[port]["timer"] > self.heartbeat_timer)):

                #  Waited too long to hear a heartbeat, notify the logs that this card might not be active

                self.known_arduinos[port]["log"].log("Arduino on COM {} isn't logging".format(port))
                self.reset_heartbeat_timer(port)

    def handle_stim_request(self, port):
        self.known_arduinos[port]["log"].log("Stimulation request received from port {}".format(port))

        stim.fire_stimulator(self.known_arduinos[port]["log"])

    def reset_heartbeat_timer(self, port):
        self.known_arduinos[port]["timer"] = time.time()

    def handle_write_request(self, data, port):
        # Data comes in as comma-separated: "RFID,force,position".

        rfid, force, pos = self.write_re_obj.search(data).group(1).split(',')
        self.known_arduinos[port]["log"].log("write request received for RFID {}".format(rfid))

        df.rat_data[rfid] = [force, pos]
        df.write_entire_data_file()

    def handle_data_request(self, data, port):
        #  Arduino is requesting the current RFID's data

        match = self.request_re_obj.search(data).group(1)
        key = match
        self.known_arduinos[port]["log"].log("Data request received for RFID {}".format(key))

        if match not in df.rat_data.keys():
            #  Match not found in the data file, send default data.

            self.known_arduinos[port]["log"].log(
                "ATTENTION: Requested RFID {} not found in data file, sending default values.".format(match)
            )
            key = "default"

            # Write the new RFID with default values to the data file
            df.write_new_data_to_existing_file(match, df.rat_data[key][0], df.rat_data[key][1])

        self.transmit_data("{},{},{}".format(match, df.rat_data[key][0], df.rat_data[key][1]), port)

    def transmit_data(self, data, port):
        #  Takes in ASCII string
        #  Converts it to binary for the micro to understand
        #  Transmits the data
        #  Lets calling code know if it was successful
        #  The Arduino is expecting the data to start with "<" and end with ">"

        data_to_transmit = "<{}>".format(data).encode()
        self.known_arduinos[port]["log"].log("Transmitting data: {}".format(data_to_transmit))

        retval = 0  # defaults to successful used if no arduino ports currently in use
        for port in self.known_arduinos.keys():
            try:
                self.known_arduinos[port]["sobj"].write(data_to_transmit)
                retval = 0  # success
            except serial.serialutil.SerialException:
                retval = 1  # failure

        return retval

cph = ComPortHandler()
