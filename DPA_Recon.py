import numpy as np
import matplotlib.pyplot as plt
import re
# from pyfinite import ffield
from sage.all import *
import random
import sys


# PoI and Parameter for F3 traces
PoI_start=58
PoI_end=1050
cycle=132
measure_offset=20
measure_range=25
v = 42
m = 28
n = m + v

# samples per trace
samp = 2000

# field size q
q = 256

x = var('x')
K = GF(q, 'a', modulus= x**8 + x**4 + x**3 + x + 1, repr = 'int')
# R = PolynomialRing(K,'x', c*v, order='degrevlex'); 
R = PolynomialRing(K,'x', v, order='degrevlex'); 
x = R.gens()

# number of bits to determine one vinegar variable
nrBits = 8

# number of candidates with high correlations for vinegar value
nrc = 10

PATH_ATTACK = ""
PATH_REF = ""

# need to check check_vin function
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
        trace_data = np.genfromtxt(PATH_REF +'reftrace_'+ hex(i) + '.csv', delimiter=',')
        ref_data[i] = trace_data.T
    # read in trace with random vinegar values for mean computation
    trace_rand_data = np.genfromtxt(PATH_REF + '/randtrace.csv', delimiter=',')
    trace_rand=trace_rand_data.T
    # compute mean of reference device (by using trace with random vinegar values)
    trace_rand_mean=np.mean(trace_rand[0:v-1,PoI_start:PoI_end],axis=0)

    return (ref_data,trace_rand_mean)


def ReplaceWithSCAoil(a_full,recoveredOil):
    for i in range(len(recoveredOil)):
        a_full[i]=recoveredOil[i]

def get_vin_cand_from_trace(inputNr,ref_data,trace_rand_mean):
    ######################################
    # Select path of traces              #
    ######################################
    path=PATH_ATTACK
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

def test_vin_cand(inputNr,indexint,indexsum,indexcomb):
    ######################################
    # Select path of traces              #
    ######################################
    path=PATH_ATTACK
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
    #print('Differences between int and sum attack at following indices',diff)

    wrong = find_different_indices(guess,vin)
    #print('This is the number of wrong vin variables in comb attack',wrong)

    ######################################
    # check first guess        #
    ######################################    
    oil = check_vin(guess,P,sig)
    if (oil):
        print('Found Oil vector for trace number',inputNr)
        return (P,oil) 
    if (len(diff)==0):
        print('We are not able to determine the oil vector with this trace')
        return 0

    ######################################
    # check if there is exactly one miss #
    ###################################### 
    for i in diff:    
        for j in range(1,nrc):
            guess[i] = K(ZZ(int(indexcomb[j,i])).digits(base=2))
            oil = check_vin(guess,P,sig)
            if (oil):
                print('Found Oil vector for trace number',inputNr)
                return (P,oil) 
        guess[i] = K(ZZ(int(indexcomb[0,i])).digits(base=2))
    if (len(diff)==1):
        print('We are not able to determine the oil vector with this trace')
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
                oil = check_vin(guess,P,sig)
                if (oil):
                    print('Found Oil vector for trace number',inputNr)
                    return (P,oil) 
        guess[subsind[0]] = K(ZZ(int(indexcomb[0,subsind[0]])).digits(base=2))
        guess[subsind[1]] = K(ZZ(int(indexcomb[0,subsind[1]])).digits(base=2))
        tries = tries + 1
    if (len(diff)==2):
        print('We are not able to determine the oil vector with this trace')
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
                    oil = check_vin(vin,P,sig)
                    if (oil):
                        print('Found Oil vector for trace number',inputNr)
                        return (P,oil) 
        guess[subsind[0]] = K(ZZ(int(indexcomb[0,subsind[0]])).digits(base=2))
        guess[subsind[1]] = K(ZZ(int(indexcomb[0,subsind[1]])).digits(base=2))
        guess[subsind[2]] = K(ZZ(int(indexcomb[0,subsind[2]])).digits(base=2))
        tries = tries + 1

    print('We are not able to determine the oil vector with this trace')
    return 0
    #print(guess)


######################################
##### helper #########################

def SplitInto_k(L, k):
    l = len(L)
    m = l // k # the length of the sublists
    return [list(L[i*m:(i+1)*m]) for i in range(k)]

def AppendIndependent(L, k, first_index):
    l = len(L)
    # l_inner=len(L[1])
    # print(L[1])
    aug_list = [[0 for j in range(k)]+L[i] for i in range(l)]
    for i in range(l):
        aug_list[i][i + first_index]=1
    return aug_list

# insert the found lin relations into the vectors		
def InsertLinEq(a_full,sol_full):
	temp = a_full[len(a_full)-1]
	for i in range(m,n):
		temp[i]=sol_full[i-m]
	a_full[len(a_full)-1] = temp

# insert the lin solutions in the list of vars to be used in the quad. system
def InsertFound(solution,vector_vars):
	for i in range(0,v-m):
		vector_vars[n-m-i-1] = solution[i]

# turn random matrix to upper diagonal
def RandomToUpper(M):
    n = M.ncols()
    retM = M
    for i in range(n):
        for j in range(i+1,n):
            retM[i,j] += retM[j,i]
            retM[j,i] = K(0) 
    return retM
            
# turn upper diagonal matrix to symmetric
def UpperToSymmetric(M):
    return M+M.transpose() 

# evaluate Multivariate map 
def Eval(F,x,y):
    return [ x*M*(y.transpose()) for M in F]

def Evalleft(F,x):
	return [ x*M for M in F]
#############################################
##### Reconciliation attack #################
# find the linear relations
def InitialLinSystem(a_full, PublicKeySymm, w):
	R = PolynomialRing(K,'x', v, order='degrevlex')
	systemM=[]
	# linear equations
	k = len(a_full)-1
	for j in range(len(a_full)-1):
		systemM += Evalleft(PublicKeySymm,Matrix(R,1,n,a_full[j]))
	A=Matrix(R,1,n,systemM[0])
	for i in range(1,len(systemM)):
		A=A.stack(vector(systemM[i])) 
	listvars=list([i for i in range(m,n)])
	listvars.reverse()
	Asmall = A[[i for i in range(len(systemM))],listvars+list([w])]
	reducedAsmall=Asmall.echelon_form()
	reversed_a_full=list([a_full[k][i] for i in range(m,n)])
	reversed_a_full.reverse()
	vector_vars=vector(reversed_a_full+list([a_full[k][w]]))
	partial_sol=reducedAsmall*vector_vars-vector([x[i] for i in range(v-1,v-m-1,-1)]) # vector should be reversed
	full_sol=[x[i] for i in range(v-m)]+[partial_sol[i] for i in range(m-1,-1,-1)]
	return full_sol, reducedAsmall, vector_vars

# solve the system in the first iteration
def SolveSystem0(system,  recoveredOil, length, reducedAsmall,vector_vars, known):
	I=ideal(system)
	gr=I.groebner_basis()
	solution_full=[]
	solution_split=[]
	temp_recoveredOil = recoveredOil
	if len(gr)==length:
		solution=[x[i]-gr[i] for i in range(length)]
		print("Oil vector found")
		InsertFound(solution,vector_vars)
		partial_sol=reducedAsmall*vector_vars-vector([x[i] for i in range(v-1,v-m-1,-1)])
		solution_split = SplitInto_k(solution, c)
		solution_full=AppendIndependent(solution_split, m, known)
		full_sol=solution_full[0]+[partial_sol[i] for i in range(m-1,-1,-1)]
		temp_recoveredOil += [full_sol]
		print(full_sol)
		solution_split[0] += [partial_sol[i] for i in range(m-1,-1,-1)]
	else:
		print("NO oil vectors found")
		if len(gr)==1:
			print("Needs randomization")
		else:
			print("Needs c+=1")
	return solution_full,solution_split,temp_recoveredOil

def InitialSystem(a_full, PublicKey, PublicKeySymm, known):
	# R = PolynomialRing(K,'x', c*v, order='invlex')
	systemM=[]
	system=[]
	# systemLin=[]
	# linear equations
	for j in range(len(a_full)):
		for k in range(max(known,j+1),len(a_full)):
			# print(j,k)
			systemM += Eval(PublicKeySymm,Matrix(R,1,n,a_full[j]), Matrix(R,1,n,a_full[k]))
	# for j in range(len(systemM)):
	# 	systemLin+=systemM[j][0]
	# I=ideal(systemLin)
	# gr=I.groebner_basis()
	# print("gr linear")
	# print(gr)
	# R = PolynomialRing(K,'x', c*v, order='degrevlex'); 
	# quadratic equations
	for j in range(known,len(a_full)):
		# print(j)
		systemM += Eval(PublicKey,Matrix(R,1,n,a_full[j]), Matrix(R,1,n,a_full[j]))
	for j in range(len(systemM)):
		system+=systemM[j][0]
	return system


def SolveSystem(system, recoveredOil,known):
	I=ideal(system)
	gr=I.groebner_basis()
	print("Groebner Basis")
	print(gr)
	solution_full=[]
	solution_split=[]
	temp_recoveredOil = recoveredOil
	if len(gr)==c*v:
		solution=[x[i]-gr[i] for i in range(c*v)]
		print(c,"additional oil vector found")
		solution_split = SplitInto_k(solution, c)
		solution_full=AppendIndependent(solution_split, m, known)
		temp_recoveredOil += solution_full
		#print(solution_full)
	else:
		print("No oil vectors found")
		if len(gr)==1:
			print("Needs randomization")
		else:
			print("Needs c+=1")
	return solution_full,solution_split,temp_recoveredOil


def reconciliation_attack(P,Oil,w):

    # c=ceil(2*n/m-2)
    # c=m # for full reconciliation in one round
    R = PolynomialRing(K,'x', v, order='degrevlex'); 
    x = list(R.gens())
    found=0
    a=SplitInto_k(x, c)
    #K = GF(q)
    #R = PolynomialRing(K,'x', c*v, order='degrevlex'); 
    #x = R.gens()
    if (w == 1):
        solution_split=[[Oil[0,i] for i in range(m,n)]]
        recoveredOil=[[Oil[0,i] for i in range(n)]]

        a_aug = solution_split + a
        a_full=AppendIndependent(a_aug, m, 0)
        ReplaceWithSCAoil(a_full,recoveredOil)

        sol_full, reducedAsmall, vector_vars = InitialLinSystem(a_full, [UpperToSymmetric(j) for j in P], w)
        InsertLinEq(a_full,sol_full)
        system = InitialSystem(a_full, P, [UpperToSymmetric(j) for j in P],w+found)
        #R = PolynomialRing(K,'x', v-m, order='degrevlex'); 
        solution_full,solution_split_found, recoveredOil = SolveSystem0(system,  recoveredOil,v-m, reducedAsmall,vector_vars,w+found)
        solution_split += solution_split_found
        found += len(solution_full)
    
    elif (w==2):
        solution_split=[[Oil[0,i] for i in range(m,n)],[Oil[1,i] for i in range(m,n)]]
        recoveredOil = [[Oil[0,i] for i in range(n)], [Oil[1,i] for i in range(n)]] ######### this is the list of input oil vectors from SCA

    else: 
        print('Input 1 or 2 oil vectors to start the reconciliation attack')

    
    print('Start the reconciliation attack with',w,'known oilvectors from the SCA')
    #print("The known oil vector")
    #print(oil)


    while found < m-w:
        print("Found ", found, "oil vectors")
        print(" ")
        a_aug = solution_split + a
        a_full=AppendIndependent(a_aug, m, 0)
        ReplaceWithSCAoil(a_full,recoveredOil)
        # print("a_full")
        # print(a_full)
        # a_full should always be of the form except when SCA vectors are replaced
        # [[1, 0, 0, a0, a1, a2, a3, a4], # known oil vector from previous round 
        #  [0, 1, 0, x0, x1, x2, x3, x4], # unknown oil vectors
        #  [0, 0, 1, x5, x6, x7, x8, x9]]
        # where ai are known, and xi unknown
        system = InitialSystem(a_full,  P, [UpperToSymmetric(j) for j in P] , w+found)
        # print("system")
        # print(system)
        #R = PolynomialRing(K,'x', v, order='degrevlex'); 
        solution_full,solution_split_found,recoveredOil = SolveSystem(system, recoveredOil, w+found)
        solution_split += solution_split_found
        found += len(solution_full)

    #print("The recovered oil space is")
    #print(recoveredOil)
    return recoveredOil

################ main ##############

# check inputs #
if(len(sys.argv) != 3):
    print("\nplease select the path [0 = prepared, 1 = generated] and the number of available traces\npython DPA_Recon [0 | 1] [number of traces]\ne.g. python DPA_Recon.py 0 25\n")
    sys.exit()
else:
    if int(sys.argv[1]) == 0:
        PATH_ATTACK = "./prepared_attacktraces/"
        PATH_REF = "./prepared_reftraces/"
    elif int(sys.argv[1]) == 1:
        PATH_ATTACK = "./gen_attacktraces/"
        PATH_REF = "./gen_reftraces/"
    else:
        print("\nplease select the path [0 = prepared, 1 = generated] and the number of available traces\npython DPA_Recon [0 | 1] [number of traces]\ne.g. python DPA_Recon.py 0 25\n")
        sys.exit()
    NUMBER_OF_TRACES = int(sys.argv[2])    

# read in ref trace #
print('Reading in reference traces')   
(ref_data,trace_rand_mean) = read_reftraces()
print('Done')  

# get oil vector from target trace #
Oilspace = zero_matrix(K,NUMBER_OF_TRACES,n)
for inputNr in range(0,NUMBER_OF_TRACES):
    print('Reading in target traces and get candidates for vin')  
    (indexint,indexsum,indexcomb) = get_vin_cand_from_trace(inputNr,ref_data,trace_rand_mean)
    print('Done')  

    print('Recover Oil Vector...')  
    (P,oil) = test_vin_cand(inputNr,indexint,indexsum,indexcomb)
    print(oil)
    # if (oil):
    #     # start reconciliation attack, as soon as it is efficient enough to work with 1 vector
    #     c = 1
    #     w = 1
    #     recoveredOil = reconciliation_attack(P,oil,w)
    #     print(recoveredOil)

    Oilspace[inputNr,:] = oil

#print(Oilspace)

# right know it is only possible to start recon with 2 known oil vectors
# start reconciliation attack with recovered oil vector(s) #
c=1
w=2
recoveredOil = reconciliation_attack(P,Oilspace,2)
print(recoveredOil)





