import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, CheckButtons

# --- 1. System Constants (Buck Converter) ---
R_nom = 6.1        # Load Resistance (Ohms)
C = 19.32e-6       # Output Cap (Farads)
L = 672e-6         # Inductor (Henries)
Vi = 12.0          # Input Voltage (V)
V_target = 6.0     # Target Output (V)

# --- 2. ESP32 Constraints (50kHz) ---
Fs = 50000.0             # 50kHz
Ts = 1.0 / Fs            # 20us period
ADC_MAX = 4095.0         # 12-bit
PWM_MAX = 1023.0         # 10-bit
V_ADC_REF = 2.45         # ADC Reference Voltage (11dB atten)
K_DIV = 1.0 / 3.13       # Voltage Divider Ratio

# Target in ADC Counts
SetPoint_Counts = (V_target * K_DIV / V_ADC_REF) * ADC_MAX

print(f"Target: {V_target}V = {int(SetPoint_Counts)} ADC Counts")

def run_simulation(Kp, Ki, Kd, use_quantization):
    # Longer time to allow Integral to settle (30ms)
    T_total = 0.030 
    steps = int(T_total / Ts)
    t = np.linspace(0, T_total, steps)
    
    v_out = np.zeros(steps)
    duty_cycle = np.zeros(steps)
    error_log = np.zeros(steps)
    
    # Initial Conditions
    v_out[0] = 0.0
    i_L = 0.0
    integral_sum = 0.0
    prev_error = 0.0
    
    # Discrete Gains
    # Ki needs to be small because it runs 50,000 times a second!
    Kp_dsp = Kp
    Ki_dsp = Ki * Ts 
    Kd_dsp = Kd / Ts

    for k in range(steps - 1):
        # 1. SENSOR (Ideal vs Quantized)
        if use_quantization:
            # Simulate real ADC steps (Integer rounding)
            adc_voltage_at_pin = v_out[k] * K_DIV
            adc_raw = int((adc_voltage_at_pin / V_ADC_REF) * ADC_MAX)
            # Clip ADC to 12-bit range
            if adc_raw > 4095: adc_raw = 4095
            if adc_raw < 0: adc_raw = 0
            current_val = adc_raw
        else:
            # Ideal floating point sensor (No steps)
            current_val = (v_out[k] * K_DIV / V_ADC_REF) * ADC_MAX

        # 2. ERROR
        error = SetPoint_Counts - current_val
        error_log[k] = error

        # 3. PID
        integral_sum += error
        
        # Anti-Windup (Clamp Integral)
        # Prevents the I-term from building up infinitely during the rise time
        integ_limit = PWM_MAX
        if integral_sum > integ_limit: integral_sum = integ_limit
        if integral_sum < -integ_limit: integral_sum = -integ_limit

        derivative = error - prev_error
        
        output = (Kp_dsp * error) + (Ki_dsp * integral_sum) + (Kd_dsp * derivative)
        
        # 4. ACTUATOR (PWM)
        # Convert PID output to Duty Cycle
        if use_quantization:
            # Real ESP32: Can only set Integer PWM counts
            pwm_counts = int(output)
            if pwm_counts > 1023: pwm_counts = 1023
            if pwm_counts < 0: pwm_counts = 0
            D = pwm_counts / PWM_MAX
        else:
            # Ideal: Floating point duty cycle
            D = output / PWM_MAX
            if D > 0.95: D = 0.95
            if D < 0.0: D = 0.0
        
        duty_cycle[k] = D
        prev_error = error
        
        # 5. PHYSICS (Buck Converter State Space)
        v_L = (Vi * D) - v_out[k]
        di_L = (v_L / L) * Ts
        i_L += di_L
        
        i_load = v_out[k] / R_nom
        dv_out = ((i_L - i_load) / C) * Ts
        v_out[k+1] = v_out[k] + dv_out
        if v_out[k+1] < 0: v_out[k+1] = 0

    return t, v_out, duty_cycle, error_log, (Kp_dsp, Ki_dsp, Kd_dsp)

# --- Plotting ---
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
plt.subplots_adjust(bottom=0.35, right=0.8)

l1, = ax1.plot([], [], lw=2, label='V_out')
ax1.axhline(V_target, color='r', linestyle='--', alpha=0.5, label='Target')
ax1.set_xlim(0, 0.030)
ax1.set_ylim(0, 8)
ax1.set_title("Response (30ms)")
ax1.grid(True)
ax1.legend()

l2, = ax2.plot([], [], color='green', lw=1)
ax2.set_xlim(0, 0.030)
ax2.set_ylim(0, 1.1)
ax2.set_title("Duty Cycle")
ax2.grid(True)

# Sliders
ax_kp = plt.axes([0.1, 0.20, 0.6, 0.03])
ax_ki = plt.axes([0.1, 0.15, 0.6, 0.03])
ax_kd = plt.axes([0.1, 0.10, 0.6, 0.03])

s_kp = Slider(ax_kp, 'Kp', 0.0, 5.0, valinit=0.8)
s_ki = Slider(ax_ki, 'Ki', 0.0, 800.0, valinit=150.0) 
s_kd = Slider(ax_kd, 'Kd', 0.0, 0.05, valinit=0.002)

# Checkbox for Reality Mode
ax_check = plt.axes([0.82, 0.15, 0.15, 0.1])
ch_quant = CheckButtons(ax_check, ['Simulate\nQuantization'], [False])

# Text Box for Code Values
text_box = plt.figtext(0.1, 0.02, "", bbox={'facecolor':'orange', 'alpha':0.2, 'pad':5})

def update(val):
    quant = ch_quant.get_status()[0]
    t, v, d, e, coeffs = run_simulation(s_kp.val, s_ki.val, s_kd.val, quant)
    
    l1.set_data(t, v)
    l2.set_data(t, d)
    
    # Calculate Steady State Error (last 10% of data)
    final_error = np.mean(e[int(-len(e)/10):])
    
    text_box.set_text(
        f"--- ESP32 CODE VALUES ---\n"
        f"float Kp = {coeffs[0]:.5f};\n"
        f"float Ki = {coeffs[1]:.5f};\n"
        f"float Kd = {coeffs[2]:.5f};\n"
        f"Final Error (Avg): {final_error:.2f} Counts"
    )
    fig.canvas.draw_idle()

s_kp.on_changed(update)
s_ki.on_changed(update)
s_kd.on_changed(update)
ch_quant.on_clicked(update)

update(None)
plt.show()