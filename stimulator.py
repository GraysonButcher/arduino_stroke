import sys
from io import StringIO
from contextlib import redirect_stdout

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
        self.eng = matlab.engine.start_matlab()
        self.eng.addpath(path, nargout=0)
        self.eng.RodStimInit(nargout=0)

    def stimulate(self):
        out = StringIO()
        err = StringIO()
        self.eng.stimulate(nargout=0, stdout=out, stderr=err)
        out_text = out.getvalue()
        err_text = err.getvalue()

        # future = self.eng.stimulate(nargout=0)
        # out = future.result()
        # print("\n\n**** WITH nargout: output is {}\n".format(out))

        # DIDN'T WORK:
        # with Capturing() as output:
        #     self.eng.stimulate(nargout=0)
        # print("stdout from Capturing() is {}".format(output))

        # f = StringIO()
        # with redirect_stdout(f):
        #     self.eng.stimulate(nargout=0)
        # print("redirect_stdout is {}".format(f.getvalue()))

        return self.evaluate_output(out_text, err_text)

    def evaluate_output(self, out, err):
        # figure out if no sensor comes back through stderr and handle logic from there.

        ret = False

        print("\nstdout is {}\nstderr is {}\n".format(out, err))
        if err != "":
            ret = False
        
        if "stimulation start success" in out:
            ret = True

        elif "Device detection failed Code:2" in out:
            ret = False
        
        return ret


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
