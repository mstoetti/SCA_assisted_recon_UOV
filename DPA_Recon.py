import numpy as np
import matplotlib.pyplot as plt
import re
# from pyfinite import ffield
from sage.all import *
import random
import sys
import time
from sage.doctest.util import Timer
from sage.misc.sage_timeit import SageTimeitResult


# PoI and Parameter for F3 traces
PoI_start = 58
PoI_end = 1050
cycle = 132
measure_offset = 20
measure_range = 25
v = 42
m = 28
n = m + v

# samples per trace
samp = 2000

# field size q
q = 256

fixed = 3*m-n

x = var('x')
# K = GF(q, 'a', modulus= x**8 + x**4 + x**3 + x + 1, repr = 'int')

F = GF(2)['y']
(y,) = F._first_ngens(1)
K = GF(q, 'z', modulus=y**8 + y**4 + y**3 + y +
       1, repr='int', proof=False, names=('z',))
(z,) = K._first_ngens(1)

# R = PolynomialRing(K,'x', c*v, order='degrevlex');
R = PolynomialRing(K, 'x', v, order='degrevlex')
x = R.gens()

# number of bits to determine one vinegar variable
nrBits = 8

# number of candidates with high correlations for vinegar value
nrc = 10

PATH_ATTACK = ""
PATH_REF = ""

############################################
# Kipnis-Shamir attack


def FindOilKipnisShamir(m, matrices_sym):
    R = PolynomialRing(K, 'x', n, order='degrevlex')
    flag_found = True
    trial = 0
    while flag_found:
        M0 = matrices_sym[0]
        M1 = matrices_sym[1]
        flag_inv = True
        while flag_inv:
            for j in range(1, m):
                M0 += K.random_element()*matrices_sym[j]
                M1 += K.random_element()*matrices_sym[j]
            if M0.is_invertible():
                flag_inv = False
        M = M0.inverse()*M1
        pol = M.charpoly()
        P = pol.factor()
        for i in range(len(P)-1, -1, -1):
            P1 = list(P)[i]
            PP = P1[0]
            PP_coef = list(PP)
            I = identity_matrix(K, n-m-fixed)
            PP_M = PP_coef[0]*I
            for ii in range(1, len(PP_coef)):
                PP_M += PP_coef[ii]*M**ii
            PP_M_Ker = PP_M.right_kernel()
            basis = PP_M_Ker.basis_matrix()
            check = Eval(matrices, Matrix(R, 1, n-m-fixed, [basis[0, i] for i in range(
                n-m-fixed)]), Matrix(R, 1, n-m-fixed, [basis[0, i] for i in range(n-m-fixed)]))
            flag1 = 0
            for i in range(m):
                if check[i][0] == 0:
                    flag1 += 1
            if flag1 == m:
                flag_found = False
                oil1 = var_change * \
                    Matrix(K, n-m-fixed, 1, [basis[0, i]
                           for i in range(n-m-fixed)])
                break
        trial += 1
    return oil1.transpose()

#############################################
##### Prep for Kipnis-Shamir attack #################

# find the linear relations


def InitialLinSystemKS(a_full, PublicKeySymm, x, fixed):
    R = PolynomialRing(K, 'x', n, order='degrevlex')
    systemM = []
    system = [x[n-m-i] for i in range(fixed, 0, -1)]
    # linear equations
    systemM = Eval(PublicKeySymm, Matrix(
        R, 1, n, a_full[0]), Matrix(R, 1, n, a_full[1]))
    for j in range(len(systemM)):
        system += systemM[j][0]
    Arev = LinearSystemToMatrixReversed(system, n, m+fixed)
    Areduced = Arev.echelon_form()
    xrev = x
    xrev.reverse()
    xrev_short = []
    for jj in range(m+fixed):
        xrev_short += [xrev[jj]]
    partial_sol = Areduced*Matrix(R, n, 1, xrev) + \
        Matrix(R, m+fixed, 1, xrev_short)
    x.reverse()
    full_sol = [x[i] for i in range(n-m-fixed)]+[partial_sol[i, 0]
                                                 for i in range(m-1+fixed, -1, -1)]
    return full_sol

# find matrix form of linear system in reversed order


def LinearSystemToMatrixReversed(system, n, m):
    R = PolynomialRing(K, 'x', n, order='degrevlex')
    x = list(R.gens())
    A = Matrix(K, m, n)
    for i in range(len(system)):
        temp = system[i]
        coefs = temp.coefficients()
        monoms = temp.monomials()
        for j in range(n):
            for l in range(len(monoms)):
                if monoms[l] == x[j]:
                    A[i, n-j-1] = coefs[l]
    return A

# find matrix form of linear system


def LinearSystemToMatrix(system, n, m):
    R = PolynomialRing(K, 'x', n, order='degrevlex')
    x = list(R.gens())
    A = Matrix(K, m, n)
    for i in range(len(system)):
        temp = system[i]
        coefs = temp.coefficients()
        monoms = temp.monomials()
        for j in range(n):
            for l in range(len(monoms)):
                if monoms[l] == x[j]:
                    A[i, j] = coefs[l]
    return A

# form the system


def InitialSystemKS(a_full, PublicKey):
    systemM = []
    system = []
    for j in range(0, len(a_full)):
        systemM += Eval(PublicKey, Matrix(R, 1, n,
                        a_full[j]), Matrix(R, 1, n, a_full[j]))
    for j in range(len(systemM)):
        system += systemM[j][0]
    return system

# find coefficient matrices


def PolynomialToMatrix(system, k):
    R = PolynomialRing(K, 'x', n, order='degrevlex')
    x = list(R.gens())
    system_matrices = []
    system_matrices_sym = []
    for i in range(len(system)):
        M = Matrix(K, k, k)
        temp = system[i]
        coefs = temp.coefficients()
        monoms = temp.monomials()
        for j1 in range(k):
            for j2 in range(j1, k):
                for l in range(len(monoms)):
                    if monoms[l] == x[j1]*x[j2]:
                        M[j1, j2] = coefs[l]
        system_matrices += [M]
        system_matrices_sym += [M+M.transpose()]
    return system_matrices, system_matrices_sym

# need to check check_vin function


def check_vin(vin, P, sig):
    oil = zero_vector(K, n)
    for i in range(n):
        if (i < v):
            oil[i] = sig[i] + vin[i]
        else:
            oil[i] = sig[i]
    # check evaluation
    eval = zero_vector(K, m)
    for i in range(m):
        eval[i] = oil*P[i]*oil
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
    ref_data = np.empty([q, v, samp])
    for i in range(q):
        try:
            trace_data = np.genfromtxt(
                PATH_REF + 'reftrace_' + hex(i) + '.csv', delimiter=',')
        except:
            print("\nReference file \"reftrace_" + hex(i) +
                  ".csv\" missing in \"" + PATH_REF + "\"\n")
            sys.exit()
        ref_data[i] = trace_data.T
    # read in trace with random vinegar values for mean computation
    try:
        trace_rand_data = np.genfromtxt(
            PATH_REF + '/randtrace.csv', delimiter=',')
    except:
        print("\nRandom trace file \"randtrace.csv\" missing in \"" + PATH_REF + "\"\n")
        sys.exit()
    trace_rand = trace_rand_data.T
    # compute mean of reference device (by using trace with random vinegar values)
    trace_rand_mean = np.mean(trace_rand[0:v-1, PoI_start:PoI_end], axis=0)

    return (ref_data, trace_rand_mean)


def ReplaceWithSCAoil(a_full, recoveredOil):
    for i in range(len(recoveredOil)):
        a_full[i] = recoveredOil[i]


def get_vin_cand_from_trace(inputNr, ref_data, trace_rand_mean):
    ######################################
    # Select path of traces              #
    ######################################
    path = PATH_ATTACK
    traces_v_name = "meanTraces_" + str(inputNr) + ".csv"
    ######################################
    # read trace from target device      #
    ######################################
    try:
        target_data = np.genfromtxt(path+traces_v_name, delimiter=',')
    except:
        print("Attack trace file \"meanTraces_" + str(inputNr) +
              ".csv\" missing in \"" + path + "\"\n")
        sys.exit()
    tartrace = target_data.T
    # compute mean of target device (by using the traces from the real vin values, which is the only option we have for the target device)
    tartrace_mean = np.mean(tartrace[0:v-1, PoI_start:PoI_end], axis=0)
    # #############################################################################
    # # Automated attack on v-values                                              #
    # #############################################################################
    indexint = np.empty([nrc, v])
    indexsum = np.empty([nrc, v])
    indexcomb = np.empty([nrc, v])
    ######################################
    # loop over vinegar variables       #
    ######################################
    for i in range(v):
        ######################################
        # prepare attack trace               #
        ######################################
        tarcurtrace = tartrace[i, PoI_start:PoI_end] - \
            tartrace_mean  # - trace_rand_mean
        # Cut out PoIs for each bit of 8-bit vinegar variable
        poi_tartrace = []
        sumpoi_tartrace = np.empty(nrBits)
        region = 0
        for s in range(0, nrBits):
            poi_tartrace = np.append(
                poi_tartrace, tarcurtrace[region+measure_offset:region+measure_offset+measure_range])
            sumpoi_tartrace[s] = sum(
                tarcurtrace[region+measure_offset:region+measure_offset+measure_range])
            region = region + cycle
        ######################################
        # prepare ref traces and correlate to attack trace #
        ######################################
        ref_trace = np.empty([PoI_end-PoI_start])
        cotarint = np.empty(q)
        cotarsum = np.empty(q)
        for j in range(0, q):
            # collect column i and subtract mean
            ref_trace = ref_data[j, i, PoI_start:PoI_end] - trace_rand_mean
            # Cut out PoIs for each bit of 8-bit vinegar variable
            poi_reftrace = []
            sumpoi_reftrace = np.empty(nrBits)
            region = 0
            for s in range(0, nrBits):
                poi_reftrace = np.append(
                    poi_reftrace, ref_trace[region+measure_offset:region+measure_offset+measure_range])
                sumpoi_reftrace[s] = sum(
                    ref_trace[region+measure_offset:region+measure_offset+measure_range])
                region = region + cycle

            # Correlate target trace with reference traces  #
            cotarint[j] = round(np.corrcoef(
                poi_tartrace, poi_reftrace)[0, 1], 2)
            cotarsum[j] = round(np.corrcoef(
                sumpoi_tartrace, sumpoi_reftrace)[0, 1], 2)

        # find maximum correlation
        cotarcomb = cotarint + cotarsum
        #index = np.argmax(cotar)
        #sumindex = np.argmax(cotarsum)
        #combindex = np.argmax(cotarcomb)
        #print(np.argpartition(cotarint, -5)[-5:])

        indexint[:, i] = cotarint.argsort()[-nrc:][::-1]
        indexsum[:, i] = cotarsum.argsort()[-nrc:][::-1]
        indexcomb[:, i] = cotarcomb.argsort()[-nrc:][::-1]

    return (indexint, indexsum, indexcomb)


def test_vin_cand(inputNr, indexint, indexsum, indexcomb):
    ######################################
    # Select path of traces              #
    ######################################
    path = PATH_ATTACK
    vinegar_file = "input_" + str(inputNr) + ".txt"
    publickey_file = "pk_sig_" + str(inputNr) + ".txt"
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
    # print(vinegars)
    vin = zero_vector(K, v)
    for i in range(v):
        vin[i] = K(ZZ(int(vinegars[i], 16)).digits(base=2))
    # print(vin)
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
    P = [zero_matrix(K, n, n) for _ in range(m)]
    t = 0
    # sort P1array to matrices
    for i in range(n):
        for j in range(i, n):
            for k in range(m):
                P[k][i, j] = K(ZZ(int(pk[t], 16)).digits(base=2))
                t = t+1
    # print(P[0])
    # print(t)
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
    # print(signature)
    sig = zero_vector(K, n)
    for i in range(n):
        sig[i] = K(ZZ(int(signature[i], 16)).digits(base=2))
    # print(sig)

    ######################################
    # try to guess vinegar variable         #
    ######################################
    tries = 0
    success = 0

    guess_vin = indexcomb[0, :]
    guess = zero_vector(K, v)
    for i in range(len(guess_vin)):
        guess[i] = K(ZZ(int(guess_vin[i])).digits(base=2))

    diff = find_different_indices(indexint[0, :], indexsum[0, :])
    #print('Differences between int and sum attack at following indices',diff)

    wrong = find_different_indices(guess, vin)
    #print('This is the number of wrong vin variables in comb attack',wrong)

    ######################################
    # check first guess        #
    ######################################
    oil = check_vin(guess, P, sig)
    if (oil):
        print('Found Oil vector for trace number', inputNr)
        return (P, oil)
    if (len(diff) == 0):
        print('We are not able to determine the oil vector with this trace\n')
        return (0, 0)

    ######################################
    # check if there is exactly one miss #
    ######################################
    for i in diff:
        for j in range(1, nrc):
            guess[i] = K(ZZ(int(indexcomb[j, i])).digits(base=2))
            oil = check_vin(guess, P, sig)
            if (oil):
                print('Found Oil vector for trace number', inputNr)
                return (P, oil)
        guess[i] = K(ZZ(int(indexcomb[0, i])).digits(base=2))
    if (len(diff) == 1):
        print('We are not able to determine the oil vector with this trace\n')
        return (0, 0)

    ######################################
    # check if there are exactly two misses #
    ######################################
    while (guess != vin) & (tries < 100):
        indices = random.sample(range(len(diff)), 2)
        subsind = [diff[i] for i in indices]
        # print(subsind)
        for j in range(0, nrc):
            for k in range(0, nrc):
                guess[subsind[0]] = K(
                    ZZ(int(indexcomb[j, subsind[0]])).digits(base=2))
                guess[subsind[1]] = K(
                    ZZ(int(indexcomb[k, subsind[1]])).digits(base=2))
                oil = check_vin(guess, P, sig)
                if (oil):
                    print('Found Oil vector for trace number', inputNr)
                    return (P, oil)
        guess[subsind[0]] = K(ZZ(int(indexcomb[0, subsind[0]])).digits(base=2))
        guess[subsind[1]] = K(ZZ(int(indexcomb[0, subsind[1]])).digits(base=2))
        tries = tries + 1
    if (len(diff) == 2):
        print('We are not able to determine the oil vector with this trace\n')
        return (0, 0)

    tries = 0
    ######################################
    # check if there are exactly three misses #
    ######################################
    while (guess != vin) & (tries < 100):
        indices = random.sample(range(len(diff)), 3)
        subsind = [diff[i] for i in indices]
        for j in range(0, nrc):
            for k in range(0, nrc):
                for l in range(0, nrc):
                    guess[subsind[0]] = K(
                        ZZ(int(indexcomb[j, subsind[0]])).digits(base=2))
                    guess[subsind[1]] = K(
                        ZZ(int(indexcomb[k, subsind[1]])).digits(base=2))
                    guess[subsind[2]] = K(
                        ZZ(int(indexcomb[l, subsind[2]])).digits(base=2))
                    oil = check_vin(vin, P, sig)
                    if (oil):
                        print('Found Oil vector for trace number', inputNr)
                        return (P, oil)
        guess[subsind[0]] = K(ZZ(int(indexcomb[0, subsind[0]])).digits(base=2))
        guess[subsind[1]] = K(ZZ(int(indexcomb[0, subsind[1]])).digits(base=2))
        guess[subsind[2]] = K(ZZ(int(indexcomb[0, subsind[2]])).digits(base=2))
        tries = tries + 1

    print('We are not able to determine the oil vector with this trace\n')
    return (0, 0)
    # print(guess)


######################################
##### helper #########################

def SplitInto_k(L, k):
    l = len(L)
    m = l // k  # the length of the sublists
    return [list(L[i*m:(i+1)*m]) for i in range(k)]


def AppendIndependent(L, k, first_index):
    l = len(L)
    # l_inner=len(L[1])
    # print(L[1])
    aug_list = [[0 for j in range(k)]+L[i] for i in range(l)]
    for i in range(l):
        aug_list[i][i + first_index] = 1
    return aug_list

# replace the beginning of the list of oil vectors by ones found using SCA


def ReplaceWithSCAoil(a_full, recoveredOil):
    for i in range(len(recoveredOil)):
        a_full[i] = recoveredOil[i]

# insert the found lin relations into the vectors


def InsertLinEq(a_full, sol_full):
    temp = a_full[len(a_full)-1]
    for i in range(m, n):
        temp[i] = sol_full[i-m]
    a_full[len(a_full)-1] = temp

# insert the lin solutions in the list of vars to be used in the quad. system


def InsertFound(solution, vector_vars):
    for i in range(0, v-m):
        vector_vars[n-m-i-1] = solution[i]

# turn random matrix to upper diagonal


def RandomToUpper(M):
    n = M.ncols()
    retM = M
    for i in range(n):
        for j in range(i+1, n):
            retM[i, j] += retM[j, i]
            retM[j, i] = K(0)
    return retM

# turn upper diagonal matrix to symmetric


def UpperToSymmetric(M):
    return M+M.transpose()

# evaluate Multivariate map


def Eval(F, x, y):
    return [x*M*(y.transpose()) for M in F]


def Evalleft(F, x):
    return [x*M for M in F]

#############################################
##### Reconciliation attack #################

# find the linear relations


def InitialLinSystem(a_full, PublicKeySymm, known):
    R = PolynomialRing(K, 'x', v, order='degrevlex')
    systemM = []
    # linear equations
    k = len(a_full)-1
    for j in range(len(a_full)-1):
        systemM += Evalleft(PublicKeySymm, Matrix(R, 1, n, a_full[j]))
    A = Matrix(R, 1, n, systemM[0])
    for i in range(1, len(systemM)):
        A = A.stack(vector(systemM[i]))
    listvars = list([i for i in range(m, n)])
    listvars.reverse()
    Asmall = A[[i for i in range(len(systemM))], listvars+list([w])]
    reducedAsmall = Asmall.echelon_form()
    reversed_a_full = list([a_full[k][i] for i in range(m, n)])
    reversed_a_full.reverse()
    vector_vars = vector(reversed_a_full+list([a_full[k][w]]))
    partial_sol = reducedAsmall*vector_vars - \
        vector([x[i] for i in range(v-1, v-m-1, -1)])
    full_sol = [x[i] for i in range(v-m)]+[partial_sol[i]
                                           for i in range(m-1, -1, -1)]
    return full_sol, reducedAsmall, vector_vars

# form the system


def InitialSystem(a_full, PublicKey, PublicKeySymm, known):
    systemM = []
    system = []
    # linear equations
    for j in range(len(a_full)):
        for k in range(max(known, j+1), len(a_full)):
            systemM += Eval(PublicKeySymm, Matrix(R, 1, n,
                            a_full[j]), Matrix(R, 1, n, a_full[k]))
    # quadratic equations
    for j in range(known, len(a_full)):
        systemM += Eval(PublicKey, Matrix(R, 1, n,
                        a_full[j]), Matrix(R, 1, n, a_full[j]))
    for j in range(len(systemM)):
        system += systemM[j][0]
    return system

# solve the system


def SolveSystem(system, recoveredOil):
    I = ideal(system)
    gr = I.groebner_basis()

    solution_full = []
    solution_split = []
    temp_recoveredOil = recoveredOil
    if len(gr) == v:
        solution = [x[i]-gr[i] for i in range(v)]

        solution_split = SplitInto_k(solution, 1)
        solution_full = AppendIndependent(solution_split, m, w + found)
        temp_recoveredOil += solution_full

    else:
        print("NO oil vectors found")
        if len(gr) == 1:
            print("Needs randomization")
        else:
            print("Needs more vectors")
    return solution_full, solution_split, temp_recoveredOil


# check inputs #
if (len(sys.argv) != 3):
    print(
        "\nPlease select the path [0 = prepared, 1 = generated] and the number of available traces\npython DPA_Recon [0 | 1] [number of traces]\ne.g. python DPA_Recon.py 0 25\n")
    sys.exit()
else:
    try:
        if int(sys.argv[1]) == 0:
            PATH_ATTACK = "./prepared_attacktraces/"
            PATH_REF = "./prepared_reftraces/"
        elif int(sys.argv[1]) == 1:
            PATH_ATTACK = "./gen_attacktraces/"
            PATH_REF = "./gen_reftraces/"
        else:
            print(
                "\nPlease select the path to the traces [0 = prepared, 1 = generated] and the number of available attacktraces\npython DPA_Recon [0 | 1] [number of traces]\ne.g. python DPA_Recon.py 0 25\n")
            sys.exit()
        NUMBER_OF_TRACES = int(sys.argv[2])
    except:
        print(
            "\nPlease select the path to the traces [0 = prepared, 1 = generated] and the number of available attacktraces\npython DPA_Recon [0 | 1] [number of traces]\ne.g. python DPA_Recon.py 0 25\n")

        sys.exit()

# read in ref trace #
print('Reading in reference traces')
(ref_data, trace_rand_mean) = read_reftraces()
print('Done')

# get oil vector from target trace #
Oilspace = zero_matrix(K, NUMBER_OF_TRACES, n)
oil_counter = 0
for inputNr in range(0, NUMBER_OF_TRACES):
    print('Reading in target traces and get candidates for vin')
    (indexint, indexsum, indexcomb) = get_vin_cand_from_trace(
        inputNr, ref_data, trace_rand_mean)
    print('Done')

    print('Recover Oil Vector...')
    (P, oil) = test_vin_cand(inputNr, indexint, indexsum, indexcomb)
    if (oil):
        Oilspace[inputNr, :] = oil
        oil_counter += 1
        print(oil, '\n')

# check if oil vector available
if oil_counter:
    # start reconciliation attack with recovered oil vector(s) #
    # read in number of traces to start with
    bad_choice = True
    while (bad_choice):
        oil_choice = input("Please select which one of the " + str(oil_counter) +
                           " recovered oil vector(s) [0,...," + str(oil_counter-1) + "] should be used for the next steps?\n > ")
        try:
            oil_choice = int(oil_choice)
        except:
            print("Please select only 1 integer value.")
            continue
        if oil_choice < 0 or oil_choice > oil_counter-1:
            print(
                "Please select a value in this range [0,...," + str(oil_counter-1) + "].")
            continue
        bad_choice = False
    Oilspace = [Oilspace[oil_choice][i] for i in range(n)]

    R = PolynomialRing(K, 'x', n, order='degrevlex')
    sol_full = InitialLinSystemKS(
        [Oilspace] + [list(R.gens())], [UpperToSymmetric(j) for j in P], list(R.gens()), fixed)

    var_change = LinearSystemToMatrix(sol_full, n-m-fixed, n)

    system = InitialSystemKS([sol_full], P)

    matrices, matrices_sym = PolynomialToMatrix(system, n-m-fixed)

    foundoil = FindOilKipnisShamir(m, matrices_sym)
    Oilspace = [Oilspace] + [[foundoil[0][i] for i in range(n)]]

    w = 2

    # start the reconciliation part
    x = R.gens()

    found = 0
    a = SplitInto_k([x[i] for i in range(v)], 1)

    solution_split = [[Oilspace[0][i]
                       for i in range(m, n)], [Oilspace[1][i] for i in range(m, n)]]

    while found < m-w:

        a_aug = solution_split + a

        a_full = AppendIndependent(a_aug, m, 0)

        ReplaceWithSCAoil(a_full, Oilspace)

        system = InitialSystem(
            a_full, P, [UpperToSymmetric(j) for j in P], w+found)

        solution_full, solution_split_found, Oilspace = SolveSystem(
            system, Oilspace)
        solution_split += solution_split_found

        found += len(solution_full)

    print('The following is a basis of the secret Oilspace.\nThis completes the key recovery.\n')
    # Convert Oilspace entries from polynomial to integer representation for output
    for i in range(w, m):

        for j in range(m, n):
            if not (Oilspace[i][j].coefficients()):
                Oilspace[i][j] = 0
            else:
                Oilspace[i][j] = Oilspace[i][j].coefficients()[0]

    for i in range(m):
        print(Oilspace[i])

else:
    print("No recovered oil vector.\n")
