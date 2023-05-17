import numpy as np
import sys

Q = 256
NUMBER_OF_ATTACK_TRACES = 100

PATH_REF = "/tmp/SCA_assisted_recon_UOV/prepared_reftraces/csv/"
PATH_REF_CONV = "/tmp/SCA_assisted_recon_UOV/prepared_reftraces/raw/"

PATH_ATTACK = "/tmp/SCA_assisted_recon_UOV/prepared_attacktraces/csv/"
PATH_ATTACK_CONV = "/tmp/SCA_assisted_recon_UOV/prepared_attacktraces/raw/"

for i in range(Q):
    try:
        trace_data = np.load(PATH_REF_CONV + 'reftrace_'+ hex(i) + '.raw', allow_pickle=True)
        
    except:
        print("\nReference file \"reftrace_" + hex(i) + ".raw\" missing in \""+ PATH_REF_CONV +"\"\n")
        sys.exit()

    np.savetxt(PATH_REF + 'reftrace_'+ hex(i) + '.csv', trace_data, delimiter=',', comments="")   

try:
    trace_data = np.load(PATH_REF_CONV + '/randtrace.raw', allow_pickle=True)
except:
    print("\nRandom trace file \"randtrace.csv\" missing in \""+ PATH_REF_CONV +"\"\n")
    sys.exit()    

np.savetxt(PATH_REF + 'randtrace.csv', trace_data, delimiter=',', comments="")  

# ############################################################

for i in range(NUMBER_OF_ATTACK_TRACES):
    try:
        trace_data = np.load(PATH_ATTACK_CONV + 'meanTraces_'+ str(i) + '.raw', allow_pickle=True)
    except:
        print("Attack trace file \"meanTraces_" + str(i) + ".raw\" missing in \""+ PATH_ATTACK_CONV +"\"\n")
        sys.exit()

    np.savetxt(PATH_ATTACK + 'meanTraces_'+ str(i) + '.csv', trace_data, delimiter=',', comments="")
