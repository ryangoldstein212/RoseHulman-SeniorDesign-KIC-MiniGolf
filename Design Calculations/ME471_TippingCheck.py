import numpy as np
from math import *
import NumericalMethods_Functions as NM


def ForceSolve(xF_vec):
    # Position Parameters
    in2ft = 1/12

    # Tipping location [in]
    xT = 0.0*in2ft
    yT = 47.0*in2ft
    zT = 0.0*in2ft

    # Applied force location [in]
    xF = xF_vec[0,0]
    yF = xF_vec[1,0]
    zF = xF_vec[2,0]

    # Lower step center of gravity location [in]
    xL = -14.31*in2ft
    yL = 46.59*in2ft
    zL = 3.90*in2ft

    # upper step center of gravity location [in]
    xU = -67.12*in2ft
    yU = 46.59*in2ft
    zU = 6.90*in2ft

    # plinko center of gravity location [in]
    xP = -86.25*in2ft
    yP = 47.37*in2ft
    zP = 39.79

    # moment location [in]
    '''
    x0 = 0*in2ft
    y0 = 0*in2ft
    z0 = 0 *in2ft
    '''
    x0 = xT
    y0 = yT
    z0 = zT

    # Inertial Terms
    g = 32.2 # acceleration due to gravity
    # masses [lbm]
    mL = 130*(1/32.2)
    mU = 273*(1/32.2)
    mP = 100*(1/32.2)
    sum_mass = mL+mP+mU
    # Moments of Inertia
    JL = 4
    JU = 3
    JP = 2
    sum_intert = JL+JP+JU

    # Rotational Accelerations
    alpx = 0
    alpy = 0
    alpz = 0 

    # Linear Accelerations
    ax = 0
    ay = 0
    az = 0

    # Linear Algebra
    A = np.array([[1, 0, 0, 1, 0, 0], [0, 1, 0, 0, 1, 0], [0, 0, 1, 0, 0, 1], [0, (z0-zF), (yF-y0), 0, (z0-zT), (yT-y0)], [(zF-z0), 0, (x0-xF), (zT-z0), 0, (x0-xT)], [0, (xF-x0), (y0-yF), (y0-yT), (xT-x0), 0]])
    b = np.array([[sum_mass*ax], [sum_mass*ay], [sum_mass*(az + g)], [sum_intert*alpx + g*((yL-y0)*mL + (yU-y0)*mU + (yP-y0)*mP)], [sum_intert*alpy + g*((x0-xL)*mL + (xU-x0)*mU + (x0-xP)*mP)], [sum_intert*alpz]])
    
    # Multiplying A transpose by A and A transpose by b
    A_trans_mult_A = np.transpose(A)@A
    A_trans_mult_b = np.transpose(A)@b
    # Solving System
    x_vec = np.linalg.solve(A_trans_mult_A, A_trans_mult_b)
    return(A, x_vec, b)

def Residual(x_guess):
    A_guess, x_vec, b_guess = ForceSolve(x_guess)
    Residual = (A_guess@x_vec) - b_guess
    return(Residual)

# Finding Critical Locations of Tip
in2ft = 1/12


# location of the tipping force
x_tip_guess_vec =  np.array([[-92.250*in2ft], [47.0*in2ft], [72.75*in2ft]])
'''
offset = 0.001
maxIterations = 1000
tol = 0.00001
x_Tip, error = NM.func_MDnewton_secant(Residual, offset, x_tip_guess_vec, tol, maxIterations, 1)

A_sol, x_vec_sol, b_sol = ForceSolve(x_Tip)
print(f"The load [lbf] required for tipping is {x_vec_sol}")
print(f"\nat location {x_Tip/in2ft} [in]")
print(f"with error {error}")
'''
A, x, b = ForceSolve(x_tip_guess_vec)
F_tip = x[0:3]
Reactions = x[4:6]
print("\n")
print(A)
print(x)
print(f"The tipping force is {np.linalg.norm(F_tip)} [lbf].")
print(b)
