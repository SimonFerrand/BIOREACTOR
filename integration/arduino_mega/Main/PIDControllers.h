#ifndef PIDCONTROLLERS_H
#define PIDCONTROLLERS_H

#include <PID_v1_bc.h>

// Define PID variables
extern double tempSetpoint, tempInput, tempOutput;
extern double phSetpoint, phInput, phOutput;
extern double doSetpoint, doInput, doOutput;

// Initialize PID controllers
extern PID tempPID;
extern PID phPID;
extern PID doPID;

void initializePIDs(double tempKp, double tempKi, double tempKd,
                    double phKp, double phKi, double phKd,
                    double doKp, double doKi, double doKd);

#endif
