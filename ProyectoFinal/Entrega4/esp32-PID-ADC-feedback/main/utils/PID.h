#ifndef PID_H
#define PID_H

typedef struct {
    // Coefficients
    float kp;
    float ki;
    float kd;
    
    // Internal Control vars
    float alpha;      // Derivative filter smoothing factor
    float beta;       // Derivative gain factor
    float ts;         // Sampling time
    
    // State
    float reference;
    float error;
    float prev_error;
    float prev_deriv;
    float integral;   // Added missing integral state
    
    // Outputs
    float proportional;
    float derivative;
    float output;
    
    // Limits
    float min_out;
    float max_out;
} pid_t;

void pid_init(pid_t *pid, float kp, float ki, float kd, float tf, float ts, float min_out, float max_out);
float pid_compute(pid_t *pid, float measurement);

#endif // PID_H