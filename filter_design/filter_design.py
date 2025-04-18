import numpy as np
import matplotlib.pyplot as plt

def H_PN(z, G, B, omega_c):
    sqrtG = np.sqrt(G)
    tanB2 = np.tan(B/2.0)
    cosomegac = np.cos(omega_c)
    n = sqrtG+G*tanB2-(2.0*sqrtG*cosomegac)/z+(sqrtG-G*tanB2)/(z*z)
    d = sqrtG+tanB2-(2.0*sqrtG*cosomegac)/z+(sqrtG-tanB2)/(z*z)
    return n/d

def frequency_response(f_min, f_max, f_c, f_B, G, n_samples):
    omega_min = f_min / f_max * np.pi
    omega_max = np.pi
    omega_c = f_c / f_max * np.pi
    B = f_B / f_max * np.pi
    x = np.linspace(omega_min, omega_max, n_samples)
    x2 = np.exp(1j*x)
    y = np.abs(np.array(H_PN(x2, G, B, omega_c)))
    x = x * f_max / np.pi
    return x, y

def main():
    G = 0.5
    f_min = 0.0
    f_max = 24000.0
    f_c = 1000.0
    f_B = f_c
    n_samples = 1000
    
    x, y = frequency_response(f_min, f_max, f_c, f_B, G, n_samples)
    fig = plt.figure(figsize=(7, 7), dpi=100)
    ax = fig.add_subplot(111)
    
    ax.plot(x, y)
    plt.xscale("log")
    plt.show()
    
if __name__ == "__main__":
    main()