import sys
from io import StringIO
import traceback
import threading

class Capturing(list):
    def __enter__(self):
        self._stdout = sys.stdout
        sys.stdout = self._stringio = StringIO()
        return self
    def __exit__(self, *args):
        self.extend(self._stringio.getvalue().splitlines())
        del self._stringio    # free up some memory
        sys.stdout = self._stdout

class Stimulator:
    def stimulate(self):
        pass

    def evaluate_output(self, out, err):
        pass

    def check_stimuator_and_log_results(self):
        pass

    def fire_stimulator(self, logger_obj):
        pass


class RealStimulator(Stimulator):
    def __init__(self, path=r'c:\users\gmb\documents\matlab'):
        print("Initializing Stimulator Connection via Matlab...")
        self.eng = matlab.engine.start_matlab()
        self.eng.addpath(path, nargout=0)
        self.eng.RodStimInit(nargout=0)
        self.stimulator_available = True
        self.stimulate_result = None

    def stimulate(self):
        out = StringIO()
        err = StringIO()
        self.eng.stimulate(nargout=0, stdout=out, stderr=err)
        out_text = out.getvalue()
        err_text = err.getvalue()

        self.stimulate_result = self.evaluate_output(out_text, err_text)

    def evaluate_output(self, out, err):
        # figure out if no sensor comes back through stderr and handle logic from there.

        ret = False

        if err != "":
            ret = False
        
        if "stimulation start success" in out:
            ret = True

        elif "Device detection failed Code:2" in out:
            ret = False
        
        return ret
    
    def check_stimuator_and_log_results(self):
        if self.stimulator_available:
            return

        if not self.stimulate_thread.is_alive():
            if self.stimulate_result:
                self.logger_obj.log("Stimulation worked!")
            else:
                self.logger_obj.log("Stimulation failed!")
            
            self.logger_obj = None
            self.stimulator_available = True


    def fire_stimulator(self, logger_obj):
        self.logger_obj = logger_obj
        if self.stimulator_available:
            self.stimulator_available = False
            self.stimulate_thread = threading.Thread(target=self.stimulate)
            self.stimulate_thread.start()
        
        else:
            self.logger_obj.log("NOTE - Stimulator unavailable at the moment")


class SimulatedStimulator(Stimulator):
    def __init__(self):
        self.stimulator_available = True
        self.stimulate_result = None

    def stimulate(self):
        import time
        time.sleep(5)
        self.stimulate_result = self.evaluate_output("", "")

    def evaluate_output(self, out, err):
        import time
        ret = False
        if (int(time.time()) % 2) == 1:
            ret = True

        return ret
    
    def fire_stimulator(self, logger_obj):
        self.logger_obj = logger_obj
        if self.stimulator_available:
            self.stimulator_available = False
            self.stimulate_thread = threading.Thread(target=self.stimulate)
            self.stimulate_thread.start()

            self.logger_obj.log("Stimulator fired")

        else:
            self.logger_obj.log("NOTE - Stimulator unavailable at the moment")
    
    def check_stimuator_and_log_results(self):
        if self.stimulator_available:
            return

        if not self.stimulate_thread.is_alive():
            if self.stimulate_result:
                self.logger_obj.log("Stimulation WORKED!")
            else:
                self.logger_obj.log("Stimulation FAILED!")
            self.logger_obj = None
            self.stimulator_available = True


try:
    import matlab.engine
    stim = RealStimulator()
except ModuleNotFoundError:
    stim = SimulatedStimulator()
except:
    print("Matlab interaction broken, exiting.")
    print(traceback.format_exc())
    exit(1)


def run_independently():
    keep_going = 'y'
    while keep_going == 'y':
        if stim.stimulate():
            print("rat seen by device")
        else:
            print("rat NOT seen by device")

        keep_going = input("Type \"y\" if you want to keep going.")

if __name__ == "__main__":
    run_independently()
