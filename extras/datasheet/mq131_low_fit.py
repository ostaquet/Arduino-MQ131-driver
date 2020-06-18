import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import numpy as np
 
def func(x, a, b, c):
    return a*(x**b)+c

x_points = np.array([1, 1.12, 1.9, 2.5, 3.8, 5.6, 7.5])
y_points = np.array([0, 10, 50, 100, 200, 500, 1000])
 
plt.scatter(x_points, y_points, c='blue', label='real data')

#give bigger weight to first and last point in real data ("force" function to fit them more precisely) 
sigma = np.ones(len(x_points))
sigma[[0, -1]] = 0.01

popt, _ = curve_fit(func, x_points, y_points, sigma=sigma)
print(popt)
 
x_show = np.arange(x_points[0], x_points[-1], 0.01)
 
plt.plot(x_show, func(x_show, *popt), c='red', label='params: a=%5.3f, b=%5.3f, c=%5.3f' % tuple(popt))
 
plt.xlabel('x')
plt.ylabel('y')
plt.legend()
plt.show()

diffs = y_points - func(x_points, *popt)
ss_tot = np.sum((y_points-np.mean(y_points))**2)
ss_res = np.sum(diffs**2)
r_to_2 = 1 - (ss_res/ss_tot)
print('R^2 score = %.10f' % r_to_2)
