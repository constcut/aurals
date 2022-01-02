#ifndef ENVELOP_H
#define ENVELOP_H

int compute_raw_envelope(
    double* cW /** Polong longr to an array of doubles, representing a signal. **/,
    unsigned int n /** Size of the array. **/,
    unsigned int mode = 0 /** mode = 0 for frontiers and mode = 1 for envelope. Envelope is returned as the positive frontier, in case of mode =1. **/
);

#endif // ENVELOP_H
