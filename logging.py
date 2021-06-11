from datetime import datetime
import os

class Log:
    #  Logger meant to be instantiated one per Arduino.
    #  The file name will be "<com port>_<year>-<month>-<day>_<hour>-<minute>-<second>.log"

    def __init__(self, comport, path=os.getcwd(), columns=[]):
        self.path = path + "\\{}_{}.log".format(comport, datetime.now().strftime("%Y-%m-%d_%H-%M-%S"))

        f = open(self.path, 'w')
        f.write("Timestamp,\n")
        [f.write("{},".format(column_name)) for column_name in columns]
        f.close()

    def log(self, msg):
        f = open(self.path, 'a')
        f.write('{},{}\n'.format(datetime.now(), msg.rstrip('\n')))
        f.close()


class PilotError(Exception):
    # Used if there's an error importing stuff
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)