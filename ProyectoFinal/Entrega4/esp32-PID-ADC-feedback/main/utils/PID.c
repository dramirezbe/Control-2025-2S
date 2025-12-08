#include "PID.h"

void pid_init(pid_t *pid, float kp, float ki, float kd, float tf, float ts, float min_out, float max_out) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->ts = ts;
    
    // Pre-calculate filter constants for efficiency
    // Alpha = Tf / (Tf + Ts)
    pid->alpha = tf / (tf + ts);
    // Beta = Kd / (Tf + Ts)
    pid->beta  = kd / (tf + ts);

    pid->min_out = min_out;
    pid->max_out = max_out;
    
    pid->prev_error = 0;
    pid->prev_deriv = 0;
    pid->integral = 0;
    pid->reference = 0;
}

float pid_compute(pid_t *pid, float measurement) {
    // 1. Error
    pid->error = pid->reference - measurement;

    // 2. Proportional
    pid->proportional = pid->kp * pid->error;

    // 3. Integral
    pid->integral += (pid->ki * pid->ts * pid->error);
    
    // Clamp Integral (Anti-windup simple)
    if (pid->integral > pid->max_out) pid->integral = pid->max_out;
    if (pid->integral < pid->min_out) pid->integral = pid->min_out;

    // 4. Derivative (with Low Pass Filter)
    // D[k] = alpha*D[k-1] + beta*(Error[k] - Error[k-1])
    pid->derivative = (pid->alpha * pid->prev_deriv) + 
                      (pid->beta * (pid->error - pid->prev_error));

    // 5. Output
    pid->output = pid->proportional + pid->integral + pid->derivative;

    // Saturation
    if (pid->output > pid->max_out) pid->output = pid->max_out;
    if (pid->output < pid->min_out) pid->output = pid->min_out;

    // Update state
    pid->prev_error = pid->error;
    pid->prev_deriv = pid->derivative;

    return pid->output;
}