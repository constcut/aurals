//This code pulled from https://github.com/tesserato/envelope

#include "envelop.h"

#include <cmath>
#include <algorithm>

#include <QDebug>


//TODO refactor this code style

struct point {
    double x /** x coordinate */;
    double y /** y coordinate */;
};


/** Given the coordinates of two points and a radius, returns the center of the circle that passes through the points and possesses the given radius.**/
point get_circle(
    double x0 /** x coordinate of the first point **/,
    double y0 /** y coordinate of the first point **/,
    double x1 /** x coordinate of the second point **/,
    double y1 /** y coordinate of the second point **/,
    double r /** radius of the circle **/
)
{
    double q{ sqrt(pow(x1 - x0, 2.0) + pow(y1 - y0, 2.0)) };
    double c{ sqrt(r * r - pow(q / 2.0, 2.0)) };

    double x3{ (x0 + x1) / 2.0 };
    double y3{ (y0 + y1) / 2.0 };
    double xc, yc;

    if (y0 + y1 >= 0) {
        xc = x3 + c * (y0 - y1) / q;
        yc = y3 + c * (x1 - x0) / q;
    }
    else {
        xc = x3 - c * (y0 - y1) / q;
        yc = y3 - c * (x1 - x0) / q;
    }
    return { xc, yc } /** Returns a point representing the center of the circle. **/;
}



/** Returns the sign of a number **/
template <typename T> long long sgn(T val /** A number (float, double, int, etc) **/) {
    return (T(0) < val) - (val < T(0))/** -1, 0 or 1, representing the sign **/;
}

/** Given a vector, returns the index of its maximum absolute value between two indices **/
template <typename T> long long argabsmax(
    const std::vector<T>& V /** Vector of numbers. **/,
    const long long x0 /** Initial index **/,
    const long long x1 /** Final index **/
)
{
    double max{ std::abs(V[x0]) };
    long long idx = x0;
    for (unsigned long long i = x0; i < x1; i++) {
        if (std::abs(V[i]) > max) {
            max = std::abs(V[i]);
            idx = i;
        }
    }
    return idx /** x0 <= idx < x1, index of the maximum absolute value of vector V between x0 and x1. **/;
}


/** Given a vector, returns the indices of the absolute maximum values of the positive and negative pulses. **/
void get_pulses(
    const std::vector<double>& W /** Vector representing the signal. **/	,
    std::vector<size_t>& posX /** Vector to be filled with the positive indices. **/,
    std::vector<size_t>& negX /** Vector to be filled with the negative indices. **/)
{
    //unsigned long long n{ W.size() };
    long long sign{ sgn(W[0]) };
    long long i0{ 0 };
    long long im{ 0 };

    posX.clear();
    negX.clear();

    for (size_t i = 1; i < W.size(); i++) {

        if (sgn(W[i]) != sign) {


            sign = sgn(W[i]);
            qDebug() << "First IF " << i - i0 << " sign " << sign;

            if (i - i0 > 4) {
                im = argabsmax(W, i0, i);
                i0 = i;

                qDebug() << "Second if " << sgn(W[im]);

                if (sgn(W[im]) >= 0)
                    posX.push_back(im);
                else
                    negX.push_back(im);
            }

        }
    }

    qDebug() << "Done " << posX.size() << " " << negX.size();
    return;
}



/** Given a vector representing the signal and a vector of the indices of its positive or negative pulses, returns the corresponding frontier. **/
std::vector<size_t> get_frontier(
    const std::vector<double>& W /** Vector representing the signal. **/,
    const std::vector<size_t>& X /** Vector of positive or negative pulses indices. **/
)
{
    unsigned long long n{ X.size() };
    double sumY{ 0.0 };
    //double sumY_vec{ W[X[n-1]] - W[X[0]] };
    //unsigned long long sumX_vec{ X[n-1] - X[0] };

    for (unsigned long long i = 0; i < n; i++) {
        sumY += W[X[i]];
    }
    double scaling{ (double(X[n - 1] - X[0]) / 2.0) / sumY };
    double sumk{ 0.0 };
    double x;
    double y;

    std::vector<double> Y(n);
    Y[0] = W[X[0]] * scaling;
    for (unsigned long long i = 1; i < n; i++) {
        Y[i] = W[X[i]] * scaling;
        x = X[i] - X[i - 1];
        y = Y[i] - Y[i - 1];
        sumk += y / (x * sqrt(x * x + y * y));
    }
    double r{ 1.0 / (sumk / (n - 1)) };
    double rr{ r * r };
    size_t idx1{ 0 };
    size_t idx2{ 1 };
    std::vector<size_t> frontierX = { X[0] };
    point pc;
    bool empty;

    while (idx2 < n) {
        pc = get_circle(X[idx1], Y[idx1], X[idx2], Y[idx2], r);
        empty = true;
        for (unsigned long long i = idx2 + 1; i < n; i++) {
            if (pow(pc.x - X[i], 2.0) + pow(pc.y - Y[i], 2.0) < rr) {
                empty = false;
                idx2 ++;
                break;
            }
        }
        if (empty) {
            frontierX.push_back(X[idx2]);
            idx1 = idx2;
            idx2 ++;
        }
    }
    return frontierX /** Vector of positive or negative frontier indices. **/;
}

std::vector<size_t> posF /** Positive frontier indices. **/,
negF /** Negative frontier indices. **/,
E /** Envelope indices. **/;

int pos_n /** Positive frontier size. **/,
neg_n /** Negative frontier size. **/;



std::vector<size_t> compute_raw_envelope(
    double* cW /** Polong longr to an array of doubles, representing a signal. **/,
    unsigned int n /** Size of the array. **/,
    unsigned int mode /** mode = 0 for frontiers and mode = 1 for envelope. Envelope is returned as the positive frontier, in case of mode =1. **/
)
{
    std::vector<double> W(cW, cW + n);
    std::vector<size_t> posX, negX;

    get_pulses(W, posX, negX);

    if (posX.size() == 0 || negX.size() == 0) {
        qDebug() << "Failed raw envelop";// "return 1;    // No pulses found
        return {};
    }

    if (mode == 0) { // Frontiers mode
        posF = get_frontier(W, posX);
        negF = get_frontier(W, negX);
    } else 	{        // Envelope mode
        E.resize(posX.size() + negX.size());
        std::set_union(posX.begin(), posX.end(), negX.begin(), negX.end(), E.begin());
        for (size_t i = 0; i < W.size(); i++) {
            W[i] = std::abs(W[i]);
        }
        posF = get_frontier(W, E);
        qDebug() << "Envelop calculated";
        return posF;
    }
    //return 0;
}
