import os
import re

class DataFile:
    def __init__(self, path=os.getcwd()):
        self.data_file = path + "\\data.txt"
        self.current_data_file_modified_timestamp = 0
        self.previous_data_file_modified_timestamp = 0
        self.error_msg = ""
        self.re_obj = re.compile("\d{17}|default,\d{1,5},\d{1,5}")

        self.default_data = "default,5,6\n12345678901234567,1,1\n96847000010331565,4,0\n96847000010332304,5,6"
        # Look for 17 characters or the word "default", one comma, a 1-5 digit number, a comma and a 1-5 digit number.

        self.rat_data = {}
        #  self.rat_data will follow the following form:
        #  {
        #      "rfid":
        #      [
        #          <force threshold>,
        #          <handle position>
        #      ]
        #  }

        self.get_data_from_file()

    def get_data_from_file(self):
        #  Checks if the file is present.
        #  Checks if the file has been updated since last checked.
        #  If so, grabs the data from the file
        #  It checks each line in the file for correct format:
        #       If correct, the row is stored and used
        #       If incorrect, the row is discarded and the program continues to the next row

        if not os.path.isfile(self.data_file):
            with open(self.data_file, 'w') as f:
                f.write(self.default_data)

        self.current_data_file_modified_timestamp = os.path.getmtime(self.data_file)
        if self.current_data_file_modified_timestamp > self.previous_data_file_modified_timestamp:
            # file has been modified, load new values
            self.load_new_values()

            if "default" not in self.rat_data.keys():
                self.error_msg += "Caution: No default specified in data file."
            self.previous_data_file_modified_timestamp = self.current_data_file_modified_timestamp

    def write_new_data_to_existing_file(self, rfid, force, pos):
        self.rat_data[rfid] = [force, pos]
        self.write_entire_data_file()

    def write_entire_data_file(self):
        with open(self.data_file, "w") as f:
            for rfid in self.rat_data.keys():
                line = "{},{},{}".format(rfid, self.rat_data[rfid][0], self.rat_data[rfid][1]).rstrip()
                f.write("{}\n".format(line))

        self.previous_data_file_modified_timestamp = self.current_data_file_modified_timestamp
        self.current_data_file_modified_timestamp = os.path.getmtime(self.data_file)

    def load_new_values(self):
        self.rat_data = {}
        row_counter = 0
        self.error_msg = ""
        with open(self.data_file, 'r') as f:
            for line in f.readlines():
                line.rstrip()
                row_counter += 1
                if self.re_obj.match(line):
                    #  Correct data found on the current row of the data file

                    values = line.split(',')
                    if values[0] in self.rat_data.keys():
                        self.error_msg += "WARNING: Skipping redundant entry found for RFID {}\n".format(values[0])
                    else:
                        self.rat_data[values[0]] = [values[1], values[2]]
                else:
                    self.error_msg += "WARNING: Error found in data file in row number {}, skipping it:\n{}" \
                        .format(row_counter, line)


df = DataFile()
