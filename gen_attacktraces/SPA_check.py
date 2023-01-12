import chipwhisperer as cw
import os
import subprocess
import sys
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime

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

_V1 = 42
_O1 = 28
N = (_V1 * _O1)/4
nMEAN = _O1/4

SCOPETYPE = 'OPENADC'
PLATFORM = 'CWLITEARM'
SS_VER='SS_VER_1_1'

CRYPTO_TARGET = "NONE"

BIN = "/uov-" + PLATFORM + ".hex"

def main(argv):
    if(len(argv) != 3):
        print(
            "\nplease specify the full path and the platform/simulation\npython UOV_SPA_check.py [path] [CWLITEARM|CW308_STM32F4|SIM] [LOOPS]\ne.g. python SPA_check.py /home/me/chipwhisperer/UOV_SPA_check/ CWLITEARM 100\n")
        sys.exit()
    else:
        PATH = argv[0]
        PLATFORM = argv[1]
        FULLPATH = PATH + BIN
        LOOPS = int(argv[2])

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



    for j in range(0, LOOPS):
        # generate inputs with given seed
        os.chdir(PATH + "/x86/")
        os.system("make clean && make")

        result = subprocess.check_output("./uov-test " + str(j), shell=True)
 
        os.chdir(PATH)

        with open(PATH + "input_" + str(j) + ".txt", 'w') as file:
            file.write(result.decode('utf8'))

        with open(PATH + "uov.input", 'r') as file :
            filedata = file.read()

        filedata = filedata.replace('<INPUT>', str(result.decode('utf8')))

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
        # tempTraces = []
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

                # tempTraces.append(scope.get_last_trace())
                mean = mean + scope.get_last_trace()

            mean = mean / (_O1//4)
            meanTraces.append(mean)

        # get calculated y 
        # y = target.read()
        # print(len(y))
        # for index in range(0, len(y)):
        #     print(hex(ord(y[index])))

        # y = target.read()
        # print(len(y))
        # for index in range(0, len(y)):
        #     print(hex(ord(y[index])))    

        meanTraces = np.array(meanTraces).T
        # tempTraces = np.array(tempTraces).T
        # tempPath = "tempTraces_" + str(j) + ".csv"
        meanPath = "meanTraces_" + str(j) + ".csv"
        # np.savetxt(path, trace, delimiter=",")
        np.savetxt(meanPath, meanTraces, delimiter=',', comments="")
        #np.savetxt(tempPath, tempTraces, delimiter=',', comments="")
        # plt.plot(trace, color='black', label='real trace')
        # plt.savefig("plt" + str(i) + ".png")
        # plt.savefig("plt" + str(i) + ".eps", format='eps', dpi=1000)
        # plt.show()    
        
    scope.dis()
    target.dis()


if __name__ == "__main__":
    main(sys.argv[1:])
