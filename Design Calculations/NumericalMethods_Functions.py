import numpy as np
import scipy
from math import *

def func_Bisection(R, xL, xR, tol, maxIter, toggle):
    if np.sign(R(xL)) == np.sign(R(xR)):
        soln = "Null"
        er_est = "Null"
        print("Provide valid, opposite sign guesses for xL and xR")
        return
    if toggle == 1:
        print("  Count     xL         R(xL)        xR         R(xR)        xG         R(xG)     Action    err_est\n")
    
    count = 0
    er_est = tol + 1
    while(er_est > tol and count < maxIter):
        count += 1
        xG = (xL + xR)/2
        er_est = abs((xL-xR)/2)
        RG = R(xG)
        RL = R(xL)
        RR = R(xR)
        if np.sign(RG) == np.sign(RL):
            xL = xG
            if toggle == 1:
                print(f" {count:3f}   {RL:10.3e}    {xR:10.3e}       {RR:10.3e}    {xG:10.3e}     {RG:10.3e}  xL = xM  {er_est:10.3e}")
        elif np.sign(RG) == np.sign(RR):
            xR = xG
            if toggle == 1:
                print(f" {count:3f}   {RL:10.3e}    {xR:10.3e}       {RR:10.3e}    {xG:10.3e}     {RG:10.3e}  xR = xM  {er_est:10.3e}")
    soln = xG
    return soln, er_est

def func_newton_secant(r, offset, xi, tol, maxIter, toggle):
    if toggle == 1:
        print("\n Count     xi+1          xi        R(xi)       dR(xi)       dx \n")
    er_est = inf
    xi_1 = xi
    count = 0
    while (er_est > tol) and (count < maxIter):
        count += 1
        xi = xi_1
        Ri = r(xi)
        x_offset = offset
        Ri_offset = r(xi + offset)
        dRdx = (Ri_offset - Ri)/offset
        dx = -Ri/dRdx
        er_est = abs(dx)
        xi_1 = xi + dx
        if toggle == 1:
            print(count, xi_1, xi, Ri, Ri_offset, dx)
    soln = xi_1
    return soln, er_est

def func_MDnewton_secant(r, offset, xi, tol, maxIter, toggle):
    if toggle == 1:
        print("\n Count   x_guess    error(inf. norm)     dR/dx     xrt -->\n")

    err_est = inf
    count = 0
    xi_1 = xi
    while (err_est > tol) and (count < maxIter):
        count += 1
        xi = xi_1
        R_xi = r(xi)
        J = np.zeros((len(R_xi), len(xi)))
        loop = 0
        for i in xi:
            x_offset = np.zeros((len(xi), 1))
            x_offset[loop] = offset
            R_offset = r(xi + x_offset)
            J[:, loop:(loop+1)] = (R_offset - R_xi)/(offset)
            loop += 1
        transposeJmultJ = np.transpose(J)@J
        transposeJmultR_xi = np.transpose(J)@R_xi

        dx = -(np.linalg.solve(transposeJmultJ, transposeJmultR_xi))
        xi_1 = xi + dx
        err_est = abs(max(dx))
        if toggle == 1:
            print(f"{count:3.0f}        {max(abs(err_est)):10.3e}         {xi[:,0]}")
        
    soln = xi_1
    return soln, err_est

def func_RK4(func_dvdt, t_span, v0, N):
    v = np.zeros([len(v0), N])
    v[:, 0:1] = v0
    vi = v0
    t = np.zeros([N, 1])
    t[0] = t_span[0]
    ti = t_span[0]
    for i in range(N):
        dt = (t_span[1] - t_span[0])/N
        t_start = t_span[0]
        t_end = t_span[1]
        # 1st Stage: Explicit Euler predictor for a half step
        k1 = func_dvdt(t_start, vi)
        vd_i12 = vi + (dt/2)*k1

        # 2nd Stage: approximate implicit Euler correctr for a half step
        k2 = func_dvdt(t_start + (dt/2), vd_i12)
        vdd_i12 = vi + (dt/2)*k2

        # 3rd Stage: mid-point predictor for a full step
        k3 = func_dvdt(t_start + (dt/2), vdd_i12)
        vddd_i1 = vi + dt*k3

        # 4th stage: funal corrector for a full step
        k4 = func_dvdt(t_start + dt, vddd_i1)
        vi = vi + dt*(k1 + 2*k2 + 2*k3 + k4)/6
        ti = ti + dt
        # Save the resulting step
        t[i] = ti
        v[:,i:(i+1)] = vi
    return t, v