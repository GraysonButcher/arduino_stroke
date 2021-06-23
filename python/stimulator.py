import sys
from io import StringIO
import traceback
from contextlib import redirect_stdout
from multiprocessing import Pool

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


class RealStimulator(Stimulator):
    def __init__(self, path=r'c:\users\gmb\documents\matlab'):
        print("Initializing Stimulator Connection via Matlab...")
        self.eng = matlab.engine.start_matlab()
        self.eng.addpath(path, nargout=0)
        self.eng.RodStimInit(nargout=0)
        self.stimulator_available = True

    def stimulate(self):
        out = StringIO()
        err = StringIO()
        self.eng.stimulate(nargout=0, stdout=out, stderr=err)
        out_text = out.getvalue()
        err_text = err.getvalue()

        return self.evaluate_output(out_text, err_text)

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

        if self.stim_call_object.ready():
            if self.stim_call_object.get():
                self.logger_obj.log("Stimulation worked!")
            else:
                self.logger_obj.log("Stimulation failed!")
            
            self.logger_obj = None
            self.stimulator_available = True


    def fire_stimulator(self, logger_obj):
        self.logger_obj = logger_obj
        if self.stimulator_available:
            pool = Pool(processes=1)
            self.stim_call_object = pool.apply_async(self.stimulate)

            self.stimulator_available = False
        
        else:
            self.logger_obj.log("NOTE - Stimulator unavailable at the moment")


class SimulatedStimulator(Stimulator):
    def __init__(self):
        self.return_state = 0
        import test as t
        self.t = t

    def stimulate(self):
        self.run_test1()
        self.run_test2()
        self.return_state += 1
        return self.return_state & 1

    def evaluate_output(self, out, err):
        pass

    def run_test1(self):
        with Capturing() as out:
            self.t.test()
        print("Capturing is:\n{}\n".format(out))

    def run_test2(self):
        f = StringIO()
        with redirect_stdout(f):
            self.t.test()
        print("redirect_stdout is:\n{}\n".format(f.getvalue()))


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
