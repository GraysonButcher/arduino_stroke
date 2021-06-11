from tkinter import filedialog
from tkinter import messagebox
from tkinter import Tk

class Flags:
    #  Used to import the header flags so values don't have to manually be synchronized between code sets
    def __init__(self):
        self.flagdict = self.getheaderdict()
        for entry in self.flagdict.keys():
            setattr(self, entry, self.flagdict[entry])

    def getheaderdict(self, filetypes=[("rat.h", "*.h")]):
        root = Tk()
        root.withdraw()
        if messagebox.askokcancel('OPEN HEADER FILE'):
            filepath = filedialog.askopenfilename(initialdir='c:\\', filetypes=filetypes, title="Select file")
            if '.h' not in filepath:
                if not messagebox.askokcancel('You chose the wrong file. Select CANCEL to find another file.'):
                    filepath = filedialog.askopenfilename(initialdir='c:\\', filetypes=filetypes, title="Select file")
            else:
                raise PilotError('ERROR: header file required.')

        root.destroy()
        root.mainloop()

        filedata = open(filepath, 'r')
        headervalues = {}
        for line in filedata.readlines():
            if '#define' in line:
                linelist = line.split()
                try:
                    headervalues[linelist[1]] = int(linelist[2])
                except ValueError:
                    if "'" in linelist[2]:
                        headervalues[linelist[1]] = linelist[2].strip("'")
                    elif '"' in linelist[2]:
                        headervalues[linelist[1]] = linelist[2].strip('"')
        filedata.close()

        return headervalues