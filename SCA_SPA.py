import numpy as np
import matplotlib.pyplot as plt
import re

# PoI and Parameter for F3 traces
PoI_start=57
PoI_end=1050
cycle=132
measure_offset=20
measure_range=30
v = 42

# PoI and Parameter for F4 traces
#PoI_start = 100
#PoI_end = 1400
#cycle = 176
#measure_offset = 0
#measure_range = 15
#v = 68

# samples per trace
samp = 2000

# field size q
q = 256

#  list of collected traces
#reflist = ['00', '01', '1C', '2A', '82', '86', 'B6', 'B8', 'EB', 'F6', 'FF']
#nr = len(reflist)

nrBits = 8

# read in reference traces
ref_data = np.empty([q,v,samp])
for i in range(q):
    trace_data = np.genfromtxt('prepared_reftraces/reftrace_'+ hex(i) + '.csv', delimiter=',')
    ref_data[i] = trace_data.T
# read in trace with random vinegar values for mean computation
trace_rand_data = np.genfromtxt('prepared_reftraces/randtrace.csv', delimiter=',')
trace_rand=trace_rand_data.T
# compute mean of reference device (by using trace with random vinegar values)
trace_rand_mean=np.mean(trace_rand[0:v-1,PoI_start:PoI_end],axis=0)



for inputNr in range(20):
    c = csum = 0
    guess_vin = [[] for _ in range(v)]
    guess_vinsum = [[] for _ in range(v)]
    #########################
    # Add path to traces    #
    #########################
    path="./prepared_attacktraces/"
    traces_v_name="meanTraces_" + str(inputNr) + ".csv"
    vinegar_file="input_" + str(inputNr) + ".txt"


    # read in vinegars for comparison
    with open(path + vinegar_file, 'r') as file:
        input = file.read()
    start = '(42)] = {'
    end = ', };'
    rx = r'42] = {(.*?), };'
    vinegars = re.findall(rx, input, re.S)
    vinegars = vinegars[0].split(", ")
    #print(vinegars)
    # read in trace from target device
    target_data = np.genfromtxt(path+traces_v_name, delimiter=',')
    tartrace = target_data.T
    # compute mean of target device (by using the traces from the real vin values, which is the only option we have for the target device) 
    tartrace_mean=np.mean(tartrace[0:v-1,PoI_start:PoI_end],axis=0)


    # #############################################################################
    # # Automated attack on v-values                                              #
    # #############################################################################

    for i in range(v):
        st=i
        
        ######################################
        # prepare target trace               #
        ######################################
        tarcurtrace = tartrace[st,PoI_start:PoI_end] - tartrace_mean
        # Cut out PoIs per v-value bit
        poi_tartrace = []
        sumpoi_tartrace = np.empty(nrBits)
        region = 0
        for s in range(0,nrBits):
            poi_tartrace = np.append(poi_tartrace,tarcurtrace[region+measure_offset:region+measure_offset+measure_range])
            sumpoi_tartrace[s] = sum(tarcurtrace[region+measure_offset:region+measure_offset+measure_range])
            region = region + cycle

        ######################################
        # prepare ref traces and correlate to attack trace #
        ######################################
        ref_trace = np.empty([PoI_end-PoI_start])
        cotar = np.empty(q)
        cotarsum = np.empty(q)
        for j in range(0,q):
            # collect column i and subtract mean
            ref_trace = ref_data[j,st,PoI_start:PoI_end] - trace_rand_mean
            # Cut out PoIs per v-value bit
            poi_reftrace = []
            sumpoi_reftrace = np.empty(nrBits)
            region = 0
            for s in range(0,nrBits):
                poi_reftrace = np.append(poi_reftrace,ref_trace[region+measure_offset:region+measure_offset+measure_range])
                sumpoi_reftrace[s] = sum(ref_trace[region+measure_offset:region+measure_offset+measure_range])
                region = region + cycle

            # Correlate target trace with reference traces  #
            cotar[j] = round(np.corrcoef(poi_tartrace,poi_reftrace)[0,1],2)
            cotarsum[j] = round(np.corrcoef(sumpoi_tartrace,sumpoi_reftrace)[0,1],2)

        # find maximum correlation
        index = np.argmax(cotar)
        sumindex = np.argmax(cotarsum)
        #ind = np.argpartition(cotar, -5)[-5:]
        guess_vin[i] = hex(index)
        guess_vinsum[i] = hex(sumindex)
        #print('Correlation of the target trace with all reference trace for vinegar value', "0x{:02X}".format(int(vinegers[i])))
        #print(cotar)
        #print('target trace has highest correlation with reference trace', hex(index))
        #print('in target trace used vinegar value', vinegars[i])
        
        if (index != int(vinegars[i],0)):
            #print('correlation of wrongly detected value', cotar[index])
            #print('correlation of true vinegar value', cotar[int(vinegars[i],0)])
            #for s in range(len(ind)):
            #    print(cotar[ind[s]], hex(ind[s]))
            c = c+1

        if (sumindex != int(vinegars[i],0)):
            #print('correlation of wrongly detected value', cotar[index])
            #print('correlation of true vinegar value', cotar[int(vinegars[i],0)])
            #for s in range(len(ind)):
            #    print(cotar[ind[s]], hex(ind[s]))
            csum = csum+1
        
    #print(guess_vin)
    print('Attack on vinegar values. Set:',inputNr,'\n')
    print('number of wrong determined variables in interval attack',c,'\n')
    print('number of wrong determined variables in sum attack',csum,'\n')



