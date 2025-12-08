import control as ct
import matplotlib.pyplot as plt
import numpy as np

# 1. System Constants
R = 6.1         # Resistance
C = 19.32e-6    # Capacitance
L = 672e-6      # Inductance
Vi = 12         # Input Voltage

# 2. PID Values (Using the "Fast Fix" values we found)
Kp = 1.5
Ki = 400
Kd = 0.00243

# 3. Define Transfer Functions
s = ct.tf('s')

# Plant: G(s) = Vi / (LCs^2 + (L/R)s + 1)
plant = Vi / (L*C*s**2 + (L/R)*s + 1)

# PID Controller: H(s) = Kp + Ki/s + Kd*s
pid = Kp + Ki/s + Kd*s

# 4. Closed Loop System
# T(s) = (Plant * PID) / (1 + Plant * PID)
G_closed_loop = ct.feedback(plant * pid, 1)

# 5. Step Response
# Simulate for 10ms (0.010 seconds)
time = np.linspace(0, 0.010, 1000)
t, y = ct.step_response(G_closed_loop, T=time)

# Scale output (Step response input is 1, but our reference is 6V)
V_ref = 6.0
y = y * V_ref 

# 6. Plot
plt.figure(figsize=(10, 6))
plt.plot(t * 1000, y, linewidth=2, label='Output Voltage')
plt.axhline(V_ref, color='r', linestyle='--', label='Target (6V)')

plt.title(f'Ideal Analog Response\n(Kp={Kp}, Ki={Ki}, Kd={Kd})')
plt.xlabel('Time (ms)')
plt.ylabel('Voltage (V)')
plt.grid(True)
plt.legend()
plt.show()