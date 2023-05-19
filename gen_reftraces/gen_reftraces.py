import chipwhisperer as cw
import os
import sys
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime
import random

#------------------------
# UOV parameter sets for
#------------------------
# STM32F3 board (reduced set) https://store.newae.com/stm32f3-target-for-cw308-arm-cortex-m4-256kb-flash-40kb-sram/
# define _GFSIZE 256
# define _V1 42
# define _O1 28
# define _O2 0
# define _HASH_LEN 32
#------------------------
# STM32F4 board (standard set NIST LEVEL 1) https://store.newae.com/stm32f4-target-for-cw308-arm-cortex-m4-1mb-flash-192kb-sram/
# define _GFSIZE 256
# define _V1 68
# define _O1 44
# define _O2 0
# define _HASH_LEN 32
#------------------------

### We reduce the parameter set of UOV, s.t. the part of the public key, which is relevant for our attack, fits on the STM32F3 board

# number of vinegar variables
_V1 = 42
# number of oil variables = number of public key equations in UOV
_O1 = 28
# number of times the target function is called for each individual vinegar variable
# pack of 4 coefficients of the public key equations are processed together 
nMEAN = _O1/4
# total number of times the target function is called during one signature generation
N = (_V1 * _O1)/4

SCOPETYPE = 'OPENADC'
PLATFORM = 'CWLITEARM'
SS_VER='SS_VER_1_1'

CRYPTO_TARGET = "NONE"

BIN = "/uov-" + PLATFORM + ".hex"

def main(argv):
    if(len(argv) != 2):
        print(
            "\nplease specify the full path and the platform\npython gen_ref_traces.py [path] [CWLITEARM|CW308_STM32F4]\ne.g. python gen_reftraces.py /home/me/chipwhisperer/gen_reftraces/ CWLITEARM\n")
        sys.exit()
    else:
        PATH = argv[0]
        PLATFORM = argv[1]
        FULLPATH = PATH + BIN
     

############################################################### %run "../../Setup_Scripts/Setup_Generic.ipynb"    
    try:
        if not scope.connectStatus:
            scope.con()
    except NameError:
        scope = cw.scope()

    try:
        if SS_VER == "SS_VER_2_1":
            target_type = cw.targets.SimpleSerial2
        elif SS_VER == "SS_VER_2_0":
            raise OSError("SS_VER_2_0 is deprecated. Use SS_VER_2_1")
        else:
            target_type = cw.targets.SimpleSerial
    except:
        SS_VER="SS_VER_1_1"
        target_type = cw.targets.SimpleSerial

    try:
        target = cw.target(scope, target_type)
    except:
        print("INFO: Caught exception on reconnecting to target - attempting to reconnect to scope first.")
        print("INFO: This is a work-around when USB has died without Python knowing. Ignore errors above this line.")
        scope = cw.scope()
        target = cw.target(scope, target_type)


    print("INFO: Found ChipWhisperer😍")

    if "STM" in PLATFORM or PLATFORM == "CWLITEARM" or PLATFORM == "CWNANO":
        prog = cw.programmers.STM32FProgrammer
    elif PLATFORM == "CW303" or PLATFORM == "CWLITEXMEGA":
        prog = cw.programmers.XMEGAProgrammer
    elif "neorv32" in PLATFORM.lower():
        prog = cw.programmers.NEORV32Programmer
    elif PLATFORM == "CW308_SAM4S":
        prog = cw.programmers.SAM4SProgrammer
    else:
        prog = None
        
    import time
    time.sleep(0.05)
    scope.default_setup()

    if PLATFORM == "CW308_SAM4S":
        scope.io.target_pwr = 0
        time.sleep(0.2)
        scope.io.target_pwr = 1
        time.sleep(0.2)
    def reset_target(scope):
        if PLATFORM == "CW303" or PLATFORM == "CWLITEXMEGA":
            scope.io.pdic = 'low'
            time.sleep(0.1)
            scope.io.pdic = 'high_z' #XMEGA doesn't like pdic driven high
            time.sleep(0.1) #xmega needs more startup time
        elif "neorv32" in PLATFORM.lower():
            raise IOError("Default iCE40 neorv32 build does not have external reset - reprogram device to reset")
        elif PLATFORM == "CW308_SAM4S":
            scope.io.nrst = 'low'
            time.sleep(0.25)
            scope.io.nrst = 'high_z'
            time.sleep(0.25)
        else:  
            scope.io.nrst = 'low'
            time.sleep(0.05)
            scope.io.nrst = 'high_z'
            time.sleep(0.05)
###############################################################            

    # generate 256 input files with vinegar variables fixed to a certain value from 0x00 to 0xFF
    #os.chdir(PATH + "/x86/")
    #os.system("make clean && make")

    #result = subprocess.check_output("./uov-test " + str(j), shell=True)

    os.chdir(PATH)

    # total number of reference traces is 257
    # we have 256 for each field element and an additional reference trace using random entries for an 'average' trace
    LOOPS = 257

    for j in range(0, LOOPS):

        with open(PATH + "uov.input", 'r') as file :
            filedata = file.read()

        if (j < 256):
            # generate reference vinegars that are all fixed to a certain value
            refvin = '{}'.format(','.join(hex(j) for _ in range(_V1)))
        else:
            # generate random vinegars
            refvin = '{}'.format(','.join(hex(random.randint(0,255)) for _ in range(_V1)))

        print(refvin)

        filedata = filedata.replace('<INPUT>', str(refvin))

        # Write the file out again
        with open(PATH + "uov.h", 'w') as file:
            file.write(filedata)
        
        os.system("make PLATFORM=" + PLATFORM + " CRYPTO_TARGET=NONE CRYPTO_OPTIONS=NONE clean")
        os.system("make PLATFORM=" + PLATFORM + " CRYPTO_TARGET=NONE CRYPTO_OPTIONS=NONE")

        fw_path = FULLPATH.format(PLATFORM)
        print(fw_path)

        cw.program_target(scope, prog, fw_path)

        print(target.read())

        samples = 2000
        counter = 0
        meanTraces = []
        tempTraces = []
        for _ in range(_V1):
            mean = np.zeros(samples)
            for _ in range(_O1//4):
                scope.adc.samples = samples
                scope.arm()

                target.write('g') 

                ret = scope.capture()
                if ret:
                    print("Target timed out!")
                    continue

                while True:
                    if target.read(1) != "r":
                        break

                tempTraces.append(scope.get_last_trace())
                mean = mean + scope.get_last_trace()

            mean = mean / (_O1//4)
            meanTraces.append(mean)

        meanTraces = np.array(meanTraces).T
        tempTraces = np.array(tempTraces).T


        if(j == 256):
            # meanPath = "randtrace.csv"
            meanPath = "randtrace.raw"
        else:
            # meanPath = "reftrace_" + hex(j) + ".csv"
            meanPath = "reftrace_" + hex(j) + ".raw"


        # np.savetxt(meanPath, meanTraces, delimiter=',', comments="")
        meanTraces.dump(meanPath)



if __name__ == "__main__":
    main(sys.argv[1:])
