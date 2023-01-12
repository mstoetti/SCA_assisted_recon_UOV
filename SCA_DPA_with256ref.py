import numpy as np
import matplotlib.pyplot as plt
import re
from pyfinite import ffield
from sage.all import *
import random

# PoI and Parameter for F3 traces
PoI_start=58
PoI_end=1050
cycle=132
measure_offset=20
measure_range=25
v = 42
m = 28
n = m + v

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

x = var('x')
K = GF(q, 'a', modulus= x**8 + x**4 + x**3 + x + 1, repr = 'int')

# number of bits to determine one vinegar variable
nrBits = 8

# number of candidates with high correlations for vinegar value
nrc = 10

def check_vin(vin,P,sig):
    oil = zero_vector(K,n)
    for i in range(n):
        if (i<v):
            oil[i] = sig[i] + vin[i]
        else:
            oil[i] = sig[i]
    # check evaluation
    eval = zero_vector(K,m)
    for i in range(m):
        eval[i]=oil*P[i]*oil
        if (eval[i] != 0):
            return 0
    return oil

def find_different_indices(arr1, arr2):
    # Initialize a list to store the indices where the arrays are different
    different_indices = []
  
    # Iterate over the elements in both arrays
    for i in range(len(arr1)):
        # If the elements are not equal, add the index to the list
        if arr1[i] != arr2[i]:
            different_indices.append(i)
  
    # Return the list of different indices
    return different_indices

def read_reftraces():
    #############################################################################
    # read reference traces from profiling device                               #
    #############################################################################
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

    return (ref_data,trace_rand_mean)

(ref_data,trace_rand_mean) = read_reftraces()

def get_vin_cand_from_trace(inputNr):
    ######################################
    # Select path of traces              #
    ######################################
    path="./prepared_attacktraces/"
    traces_v_name="meanTraces_" + str(inputNr) + ".csv"
    ######################################
    # read trace from target device      #
    ######################################
    target_data = np.genfromtxt(path+traces_v_name, delimiter=',')
    tartrace = target_data.T
    # compute mean of target device (by using the traces from the real vin values, which is the only option we have for the target device) 
    tartrace_mean=np.mean(tartrace[0:v-1,PoI_start:PoI_end],axis=0)
    # #############################################################################
    # # Automated attack on v-values                                              #
    # #############################################################################
    indexint = np.empty([nrc,v])
    indexsum = np.empty([nrc,v])
    indexcomb = np.empty([nrc,v])
    ######################################
    # loop over vinegar variables       #
    ######################################
    for i in range(v):
        ######################################
        # prepare attack trace               #
        ######################################
        tarcurtrace = tartrace[i,PoI_start:PoI_end] - tartrace_mean #- trace_rand_mean
        # Cut out PoIs for each bit of 8-bit vinegar variable
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
        cotarint = np.empty(q)
        cotarsum = np.empty(q)
        for j in range(0,q):
            # collect column i and subtract mean
            ref_trace = ref_data[j,i,PoI_start:PoI_end] -  trace_rand_mean
            # Cut out PoIs for each bit of 8-bit vinegar variable
            poi_reftrace = []
            sumpoi_reftrace = np.empty(nrBits)
            region = 0
            for s in range(0,nrBits):
                poi_reftrace = np.append(poi_reftrace,ref_trace[region+measure_offset:region+measure_offset+measure_range])
                sumpoi_reftrace[s] = sum(ref_trace[region+measure_offset:region+measure_offset+measure_range])
                region = region + cycle

            # Correlate target trace with reference traces  #
            cotarint[j] = round(np.corrcoef(poi_tartrace,poi_reftrace)[0,1],2)
            cotarsum[j] = round(np.corrcoef(sumpoi_tartrace,sumpoi_reftrace)[0,1],2)

        # find maximum correlation
        cotarcomb = cotarint +cotarsum
        #index = np.argmax(cotar)
        #sumindex = np.argmax(cotarsum)
        #combindex = np.argmax(cotarcomb)
        #print(np.argpartition(cotarint, -5)[-5:])
 
        indexint[:,i] = cotarint.argsort()[-nrc:][::-1]
        indexsum[:,i] = cotarsum.argsort()[-nrc:][::-1]
        indexcomb[:,i] = cotarcomb.argsort()[-nrc:][::-1]

    return (indexint,indexsum,indexcomb)

(indexint,indexsum,indexcomb) = get_vin_cand_from_trace(1)

def test_vin_cand(inputNr,indexint,indexsum,indexcomb):
    ######################################
    # Select path of traces              #
    ######################################
    path="./prepared_attacktraces/"
    vinegar_file="input_" + str(inputNr) + ".txt"
    publickey_file="pk_sig_" + str(inputNr) + ".txt"
    ######################################
    # read in vinegars for comparison    #
    ######################################
    with open(path + vinegar_file, 'r') as file:
        input = file.read()
    start = '(42)] = {'
    end = ', };'
    rx = r'42] = {(.*?), };'
    vinegars = re.findall(rx, input, re.S)
    vinegars = vinegars[0].split(", ")
    #print(vinegars)
    vin = zero_vector(K,v)
    for i in range(v):
        vin[i]=K(ZZ(int(vinegars[i],16)).digits(base=2))
    #print(vin)
    ######################################
    # read in public key                 #
    ######################################
    with open(path + publickey_file, 'r') as file:
        input = file.read()
    start = '(69580)] = {'
    end = ', };'
    rx = r'69580] = {(.*?), };'
    pk = re.findall(rx, input, re.S)
    pk = pk[0].split(", ")
    ######################################
    # sort PK to matrices                #
    ######################################
    P = [zero_matrix(K,n,n) for _ in range(m)]
    t = 0
    # sort P1array to matrices
    for i in range(n):
        for j in range (i,n):
            for k in range(m):
                P[k][i,j]=K(ZZ(int(pk[t],16)).digits(base=2))
                t = t+1
    #print(P[0])
    #print(t)
    ######################################
    # read in signature                  #
    ######################################
    with open(path + publickey_file, 'r') as file:
        input = file.read()
    start = '(86)] = {'
    end = ', };'
    rx = r'86] = {(.*?), };'
    signature = re.findall(rx, input, re.S)
    signature = signature[0].split(", ")
    #print(signature)
    sig = zero_vector(K,n)
    for i in range(n):
        sig[i]=K(ZZ(int(signature[i],16)).digits(base=2))
    #print(sig)

    ######################################
    # try to guess vinegar variable         #
    ######################################
    tries = 0
    success = 0

    guess_vin = indexcomb[0,:]
    guess = zero_vector(K,v)
    for i in range(len(guess_vin)):
        guess[i] = K(ZZ(int(guess_vin[i])).digits(base=2))

    diff = find_different_indices(indexint[0,:],indexsum[0,:])
    print('Differences between int and sum attack at following indices',diff)

    wrong = find_different_indices(guess,vin)
    print('This is the number of wrong vin variables in comb attack',wrong)

    ######################################
    # check first guess        #
    ######################################    
    if (guess == vin):
        success = 1
        print('Found Correct vinegar variables for InputNr',inputNr)
        return guess
    if (len(diff)==0):
        print('0We are not able to determine the oil vector with this trace')
        return 0

    ######################################
    # check if there is exactly one miss #
    ###################################### 
    for i in diff:    
        for j in range(1,nrc):
            guess[i] = K(ZZ(int(indexcomb[j,i])).digits(base=2))
            if (guess == vin):
                success = 1
                print('1Found Correct vinegar variables for InputNr',inputNr)
                return guess 
        guess[i] = K(ZZ(int(indexcomb[0,i])).digits(base=2))
    if (len(diff)==1):
        print('1We are not able to determine the oil vector with this trace')
        return 0

    ######################################
    # check if there are exactly two misses #
    ###################################### 
    while (guess != vin) & (tries < 100):
        indices = random.sample(range(len(diff)),2)
        subsind = [diff[i] for i in indices]
        #print(subsind)
        for j in range(0,nrc):
            for k in range(0,nrc):
                guess[subsind[0]] = K(ZZ(int(indexcomb[j,subsind[0]])).digits(base=2))
                guess[subsind[1]] = K(ZZ(int(indexcomb[k,subsind[1]])).digits(base=2))
                if (guess == vin):
                    success = 1
                    print('2Found Correct vinegar variables for InputNr',inputNr)
                    return guess 
        guess[subsind[0]] = K(ZZ(int(indexcomb[0,subsind[0]])).digits(base=2))
        guess[subsind[1]] = K(ZZ(int(indexcomb[0,subsind[1]])).digits(base=2))
        tries = tries + 1
    if (len(diff)==2):
        print('2We are not able to determine the oil vector with this trace')
        return 0

    tries = 0
    ######################################
    # check if there are exactly three misses #
    ######################################       
    while (guess != vin) & (tries < 100):
        indices = random.sample(range(len(diff)),3)
        subsind = [diff[i] for i in indices]
        for j in range(0,nrc):
            for k in range(0,nrc):
                for l in range(0,nrc):
                    guess[subsind[0]] = K(ZZ(int(indexcomb[j,subsind[0]])).digits(base=2))
                    guess[subsind[1]] = K(ZZ(int(indexcomb[k,subsind[1]])).digits(base=2))
                    guess[subsind[2]] = K(ZZ(int(indexcomb[l,subsind[2]])).digits(base=2))
                    if (guess == vin):
                        success = 1
                        print('3Found Correct vinegar variables for InputNr',inputNr)
                        return guess 
        guess[subsind[0]] = K(ZZ(int(indexcomb[0,subsind[0]])).digits(base=2))
        guess[subsind[1]] = K(ZZ(int(indexcomb[0,subsind[1]])).digits(base=2))
        guess[subsind[2]] = K(ZZ(int(indexcomb[0,subsind[2]])).digits(base=2))
        tries = tries + 1

    print('3We are not able to determine the oil vector with this trace') 
    #print(guess)
    
    


for inputNr in range(0,25):
    (indexint,indexsum,indexcomb) = get_vin_cand_from_trace(inputNr)
    print(test_vin_cand(inputNr,indexint,indexsum,indexcomb))
######################################
# loop over multiple attack traces to determine success probability              #
######################################

        #indexsum = np.argpartition(cotarsum, -5)[-5:]
        #combind = np.argpartition(cotarcomb, -5)[-5:]
        #guess_vin[i] = hex(index)
        #guess_vinsum[i] = hex(sumindex)
        #print('Correlation of the target trace with all reference trace for vinegar value', "0x{:02X}".format(int(vinegers[i])))
        #print(cotar)
        #print('target trace has highest correlation with reference trace', hex(index))
        #print('in target trace used vinegar value', vinegars[i])


    #if (check_vin(guess_vin,P,sig)):
    #    print('These are the correct vinegar values')
        #if (indexint != int(vinegars[i],0)):
            #print('in target trace used vinegar value', vinegars[i])
            #print('correlation of wrongly detected value', cotar[index])
            #print('correlation of true vinegar value', cotar[int(vinegars[i],0)])
            #for s in range(len(ind)):
            #    print(cotar[ind[s]], hex(ind[s]))
        #    c = c+1

        #if (indexsum != int(vinegars[i],0)):
            #print('in target trace used vinegar value', vinegars[i])
            #print('correlation of wrongly detected value', cotarsum[sumindex])
            #print('correlation of true vinegar value', cotarsum[int(vinegars[i],0)])
            #for s in range(len(sumind)):
            #    print(cotarsum[sumind[s]], hex(sumind[s]))
        #    csum = csum+1

        #if (indexcomb != int(vinegars[i],0)):
            #print('in target trace used vinegar value', vinegars[i])
            #print('correlation of wrongly detected value', cotarcomb[combindex])
            #print('correlation of true vinegar value', cotarcomb[int(vinegars[i],0)])
            #for s in range(len(combind)):
            #    print(cotarcomb[combind[s]], hex(combind[s]))
            #comb = comb+1
        
    #print(guess_vin)
    #print('Attack on vinegar values. Set:',inputNr,'\n')
    #print('number of wrong determined variables in interval attack',c,'\n')
    #print('number of wrong determined variables in sum attack',csum,'\n')
    #print('number of wrong determined variables in combined attack attack',comb,'\n')




