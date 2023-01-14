from pyfinite import ffield
import numpy as np
import random
import scipy.stats

# m = number of equations in UOV, also number of matrices F_i in the private key
# nv = number of vinegar variables

# choose parameter set
param1 = (nv,m,q) = (68,44,8)
#param2 = (nv,m,q) = (112,72,8)
#param3 = (nv,m,q) = (148,96,8)


F = ffield.FField(q) # create the field GF(2^q)

# generate the upper triangular nv x nv Matrices F1_i that are part of the private key
F1 = np.zeros((m,nv,nv), dtype=int)
for i in range(m):
	F1[i] = np.random.randint(0,2**q-1,(nv,nv))
F1 = np.triu(F1,0)

# generate several sets of random vinegar variables
v = np.zeros((1,nv), dtype=int)
v = np.random.randint(0,2**q-1,(1,nv))

# declare variables for recovered vinegars and secret key
recv = np.zeros((1,nv), dtype=int)

# create lists of elements with HW 0,1,2,...,q
listr = [ [] for _ in range(q+1) ]	
for i in range (2**q):
	listr[bin(i).count("1")].append(i)
	

# On input of F1 and v, this function replicates the batch-quad-trimat-... function (the target of our SCA), 
# that computes the product v^t * F1_i * v for i=1,...,m in the reference implementation of UOV during signing
# and outputs the HW of the intermediate products that we might observe by performing the measurements
def measure_one_sig(F1,v):
	# declare memory to safe 'measured' Hamming Weights that appear during computation
	M1 = np.zeros((m,nv,nv), dtype=int)
	M2 = np.zeros((m,nv), dtype=int)

	# replicate batch-quad-trimat-... function
	y = np.zeros((m), dtype=int)
	for j in range(nv):
		temp = np.zeros((m), dtype=int)
		# multiply the entries of the j-th row of F1_i with the j-th vinegar variable and accumulate the result in temp
		for k in range(j,nv):
			# loop over all matrices F1_i of the private key	
			for i in range(m):
				temp[i] = F.Add(temp[i], F.Multiply(F1[i,j,k],v[k]) )
				# Output and safe the HW to simulate 'noise-free' SCA of the computation
				# We can assume that we can measure the HW of each summand F.Multiply(F1[i,j,k],v[k]), since this equals the number of bits that are flipped in each accumulation process.
				M1[i,j,k] = bin(F.Multiply(F1[i,j,k],v[k])).count("1")
		# The result of this scalar product is directly multiplied with the corresponding vinegar variable and accumulated in the output vector y
		for i in range(m):
			y[i] = F.Add(y[i],F.Multiply(temp[i],v[j]))
			# Output and safe the HW to simulate 'noise-free' SCA of the computation
			# We can assume that we can measure the HW of each summand F.Multiply(temp[i],v[k]), since this equals the number of bits that are flipped in each accumulation process.
			M2[i,j] = bin(F.Multiply(temp[i],v[j])).count("1")
	return (M1,M2)

# input: field elements a,x and list of fieldelements l2
# return 1 if a*x is in l2 
# return 0 otherwise	
def count_miss(a,l2,x):
	if (F.Multiply(a,x) in l2):
		return 0
	return 1
	
# simulate noise of HW measurements 
# deviation is parameter to adjust noise level 
def sim_HWnoise(M,dev):
	size = M.shape
	errorarray = np.random.normal(0,dev,size)
	rounderror = errorarray.astype(int)
	#print('number of wrong HW measurements in the m measurements(64) needed for every vin var')
	#print(np.count_nonzero(rounderror))
	Mnoise = M + rounderror
	Mnoise[Mnoise<0]=0
	Mnoise[Mnoise>q]=q	
	return Mnoise

	
	
# analyze measurements under the condition that the entries of F1 are known (relasitic assumption, since F1 = P1)	
def analyze_measures(M1,M2,F1,noise):
	# loop over the columns of the secret matrices F1 and the corresponding vinegar variable
	# start with the last column v
	# declare candidates for vinegar entries
	recv = np.zeros(nv, dtype=int)
	#print('number of HW measurements to recover one vinegar variable')
	#print(m)
	c = 0
	for b in range(nv):
		### recover entry number b of v ###
		# store the required measurements in a new list (at the beginning only work with HW(a_ii*v_i)^k)
		m1 = M1[:,nv-1-b,nv-1-b]
		#print(m1)

		### add noise to the measurements ###
		n1 = sim_HWnoise(m1,noise)
		c = c + np.count_nonzero(n1-m1)
		z = np.zeros(2**q, dtype=int)
		#print(F1[:,nv-1-b,nv-1-b])
		for l in range(2**q):	
			for t in range(m):
				z[l] = z[l] + count_miss(F1[t,nv-1-b,nv-1-b],listr[n1[t]],l)
		#print(z)
		# the field element that maps the alpahs most often to the measured HW of the product is the best guess for the vinegar variable
		recv[nv-1-b] = np.argmin(z)	
		
	#print('average number of false HW measurements (out of m)')
	#print(np.round(c/nv,2))
	return recv

# number of trials
x = y = 0
z = 0
n = 30	 
std = 1.4
print('number of HW measurements to recover one vinegar variable')
print('m=',m)
print('number of vinegar variables')
print('v=',nv)
print('chosen standard deviation')
print('std=',std)

acc = scipy.stats.norm(0, std/2).cdf(0.5) - scipy.stats.norm(0, std/2).cdf(-0.5)
print('percentage of correctly measured Hamming weights')
print('acc=',acc)
for _ in range(n):
	###		simulate measurements		###
	M1 = np.full((m,nv,nv),0)
	M2 = np.full((m,nv),0)
	(M1,M2) = measure_one_sig(F1,v[0])
	#print(M1[1,:,:])

	# Run test, standard deviation of noise in last entry	
	recv = analyze_measures(M1,M2,F1,std)	

	#print('recovered vinegars')
	#print(recv)	
	#print('true vinegars')	
	#print(v[0])	
	x = x + np.count_nonzero(recv - v[0])
	y = y + nv
	#print('number of wrongly determined vinegar variables')
	#print(np.count_nonzero(recv - v[0]), ' of ',nv)
	if (np.count_nonzero(recv - v[0])):
		z = z +1

print('number of wrongly determined vinegar variables')
print(x, ' of ',y)
print('accuracy for one vinegar variable in %')
print(100 - np.round(x/y,4)*100)


print('number of wrongly determined vinegar vectors')
print(z, ' of ',n)
print('accuracy for one vinegar vector in %')
print(100 - np.round(z/n,4)*100)





