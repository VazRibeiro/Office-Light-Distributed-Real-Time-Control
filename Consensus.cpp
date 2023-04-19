#include <vector>
#include "Consensus.h"

/*
double rho = 0.07;

// FUNCTION TO CHECK SOLUTION FEASIBILITY
bool checkFeasibility(Node node, vector<double> d){ 
    double tol = 0.001; // tolerance for rounding errors
    if (d[node.index] < 0 - tol) {
        return false;
    }
    if (d[node.index] > 100 + tol) {
        return false;
    }
    if (inner_product(d.begin(), d.end(), node.k.begin(), 0.0) < node.L - node.o - tol) {
        return false;
    }
    return true;
};

// FUNCTION TO COMPUTE THE AUGMENTED LAGRANGIAN COST AT A POSSIBLE SOLUTION
// USED BY CONSENSUS_ITERATE
double evaluateCost(Node node, vector<double> d, double rho){
    double cost = 0;
    int n = d.size();
    for (int i = 0; i < n; i++) {
        cost += node.c[i] * d[i] + node.y[i] * (d[i] - node.d_av[i]);
    }
    cost += rho / 2 * pow(norm(d - node.d_av), 2);
    return cost;
};

// FUNCTION TO COMPUTE THE PRIMAL SOLUTION
vector<double> consensus_iterate(Node node, double rho){
    std::vector<double> d_best = {-1, -1};
    double cost_best = 1000000; // large number
    bool sol_unconstrained = true;
    bool sol_boundary_linear = true;
    bool sol_boundary_0 = true;
    bool sol_boundary_100 = true;
    bool sol_linear_0 = true;
    bool sol_linear_100 = true;
    vector<double> z(node.d_av.size());
    for (int i = 0; i < node.d_av.size(); i++){
        z[i] = rho * node.d_av[i] - node.y[i] - node.c[i];
    }

    // unconstrained minimum
    std::vector<double> d_u(node.d_av.size());
    for (int i = 0; i < node.d_av.size(); i++){
        d_u[i] = (1 / rho) * z[i];
    }
    sol_unconstrained = check_feasibility(node, d_u);
    if (sol_unconstrained){
        // IF UNCONSTRAINED SOLUTION EXISTS, THEN IT IS OPTIMAL
        // NO NEED TO COMPUTE THE OTHER
        double cost_unconstrained = evaluate_cost(node, d_u, rho);
        if (cost_unconstrained <= cost_best){
            d_best = d_u;
            cost_best = cost_unconstrained;
            return d_best, cost_best;
        }
    }

    // compute minimum constrained to linear boundary
    std::vector<double> d_bl(node.d_av.size());
    double numerator = 0;
    double denominator = 0;
    for (int i = 0; i < node.d_av.size(); i++){
        numerator += z[i] * node.k[i];
        denominator += node.k[i] * node.k[i];
    }
    double k_term = node.k[node.index] * z[node.index] - 
                    inner_product(node.k.begin(), node.k.end(), z.begin(), 0.0);
    for (int i = 0; i < node.d_av.size(); i++){
        d_bl[i] = (1 / rho) * z[i] - (node.k[i] / node.n) *
                (node.o - node.L + (1 / rho) * numerator / denominator) +
                (1 / rho / node.n) * k_term * node.k[i] / denominator;
    }
    // check feasibility of minimum constrained to linear boundary
    sol_boundary_linear = check_feasibility(node, d_bl);
    // compute cost and if best store new optimum
    if (sol_boundary_linear){
        double cost_boundary_linear = evaluate_cost(node, d_bl, rho);
        if (cost_boundary_linear < cost_best){
            d_best = d_bl;
            cost_best = cost_boundary_linear;
        }
    }

    // compute minimum constrained to 0 boundary
    std::vector<double> d_b0(node.d_av.size());
    for (int i = 0; i < node.d_av.size(); i++){
        d_b0[i] = (1 / rho) * z[i];
    }
    d_b0[node.index] = 0;
    // check feasibility of minimum constrained to 0 boundary
    sol_boundary_0 = check_feasibility(node, d_b0);
    // compute cost and if best store new optimum
    if (sol_boundary_0){
        double cost_boundary_0 = evaluate_cost(node, d_b0, rho);
        if (cost_boundary_0 < cost_best){
            d_best = d_b0;
            cost_best = cost_boundary_0;
        }
    }

    // compute minimum constrained to 100 boundary
    std::vector<double> d_b1(node.d_av.size());
    for (int i = 0; i < node.d_av.size(); i++){
        d_b1[i] = (1 / rho) * z[i];
    }
    d_b1[node.index] = 100;
    // check feasibility of minimum constrained to 100 boundary
    sol_boundary_100 = check_feasibility(node, d_b1);
    // compute cost and if best store new optimum
    if (sol_boundary_100){
        double cost_boundary_100 = evaluate_cost(node, d_b1, rho);
        if (cost_boundary_100 < cost_best){
            d_best = d_b1;
            cost_best = cost_boundary_100;
        }
    }

    // compute minimum constrained to linear and 0 boundary
    std::vector<double> d_l0 = (1/rho)*z - 
                     (1/node.m)*node.k*(node.o-node.L) +
                     (1/rho/node.m)*node.k*(node.k[node.index]*z[node.index]-
                                            dot(z,node.k));
    d_l0[node.index] = 0;
    // check feasibility of minimum constrained to linear and 0 boundary
    bool sol_linear_0 = check_feasibility(node, d_l0);
    // compute cost and if best store new optimum
    if (sol_linear_0) {
        double cost_linear_0 = evaluate_cost(node, d_l0, rho);
        if (cost_linear_0 < cost_best) {
            d_best = d_l0;
            cost_best = cost_linear_0;
        }
    }

    // compute minimum constrained to linear and 100 boundary
    std::vector<double> d_l1 = (1/rho)*z -
                     (1/node.m)*node.k*(node.o-node.L+100*node.k[node.index]) +
                     (1/rho/node.m)*node.k*(node.k[node.index]*z[node.index]-
                                            dot(z,node.k));
    d_l1[node.index] = 100;
    // check feasibility of minimum constrained to linear and 0 boundary
    bool sol_linear_1 = check_feasibility(node, d_l1);
    // compute cost and if best store new optimum
    if (sol_linear_1) {
        double cost_linear_1 = evaluate_cost(node, d_l1, rho);
        if (cost_linear_1 < cost_best) {
            d_best = d_l1;
            cost_best = cost_linear_1;
        }
    }
    
    return d_best, cost_best;
};

vector<double> consensus(Node node, vector<double> d, double rho){

    for (iterations=1; iterations<50;iterations++){
        node.d, node.c = consensus_iterate(node, rho);
    }
}
*/