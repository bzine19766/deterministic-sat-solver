%%writefile pagerank_sat_solver.cpp
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <set>
#include <map>
#include <queue>
#include <cstring>
#include <unordered_set>

using namespace std;
using namespace chrono;

// ==================== ABLATION FLAGS ====================
// Compile with -DANCHOR_LOCKING=0 etc.
#ifndef ANCHOR_LOCKING
#define ANCHOR_LOCKING 1
#endif

#ifndef ADAPTIVE_MUTATION
#define ADAPTIVE_MUTATION 1
#endif

#ifndef CLAUSE_WEIGHTING
#define CLAUSE_WEIGHTING 1
#endif

#ifndef LAST_CLAUSE
#define LAST_CLAUSE 1
#endif

#ifndef TABU_SEARCH
#define TABU_SEARCH 1
#endif

#ifndef CYCLE_DETECTION
#define CYCLE_DETECTION 1
#endif

#ifndef FREQUENCY_BIAS
#define FREQUENCY_BIAS 1
#endif

// ============================================================================
// PREPROCESSOR
// ============================================================================

class Preprocessor {
public:
    int n_vars;
    vector<vector<int>> clauses;
    vector<bool> eliminated_var;
    vector<int> var_value;

    Preprocessor(const vector<vector<int>>& cls, int n) : clauses(cls), n_vars(n) {
        eliminated_var.resize(n + 1, false);
        var_value.resize(n + 1, -1);
    }

    bool unit_propagation() {
        bool changed = true;
        while (changed) {
            changed = false;
            vector<int> units;

            for (const auto& c : clauses) {
                int unassigned = 0;
                int last_lit = 0;
                bool satisfied = false;

                for (int lit : c) {
                    int v = abs(lit);
                    if (eliminated_var[v]) continue;
                    if (var_value[v] == -1) {
                        unassigned++;
                        last_lit = lit;
                    } else if ((lit > 0 && var_value[v] == 1) || (lit < 0 && var_value[v] == 0)) {
                        satisfied = true;
                        break;
                    }
                }

                if (!satisfied && unassigned == 1) {
                    units.push_back(last_lit);
                }
            }

            for (int lit : units) {
                int v = abs(lit);
                if (var_value[v] != -1) continue;

                var_value[v] = (lit > 0) ? 1 : 0;
                changed = true;

                vector<vector<int>> new_clauses;
                for (const auto& c : clauses) {
                    bool clause_satisfied = false;
                    vector<int> new_c;

                    for (int l : c) {
                        int v2 = abs(l);
                        if (var_value[v2] == -1) {
                            new_c.push_back(l);
                        } else if ((l > 0 && var_value[v2] == 1) || (l < 0 && var_value[v2] == 0)) {
                            clause_satisfied = true;
                            break;
                        }
                    }

                    if (!clause_satisfied && !new_c.empty()) {
                        new_clauses.push_back(new_c);
                    }
                }
                clauses = move(new_clauses);
            }
        }
        return true;
    }

    bool pure_literal_elimination() {
        vector<int> pos_count(n_vars + 1, 0);
        vector<int> neg_count(n_vars + 1, 0);

        for (const auto& c : clauses) {
            for (int lit : c) {
                int v = abs(lit);
                if (eliminated_var[v]) continue;
                if (lit > 0) pos_count[v]++;
                else neg_count[v]++;
            }
        }

        bool changed = false;
        for (int v = 1; v <= n_vars; ++v) {
            if (eliminated_var[v]) continue;

            if (pos_count[v] > 0 && neg_count[v] == 0) {
                var_value[v] = 1;
                eliminated_var[v] = true;
                changed = true;
            } else if (neg_count[v] > 0 && pos_count[v] == 0) {
                var_value[v] = 0;
                eliminated_var[v] = true;
                changed = true;
            }
        }

        if (changed) {
            vector<vector<int>> new_clauses;
            for (const auto& c : clauses) {
                bool satisfied = false;
                vector<int> new_c;
                for (int lit : c) {
                    int v = abs(lit);
                    if (eliminated_var[v]) {
                        if ((lit > 0 && var_value[v] == 1) || (lit < 0 && var_value[v] == 0)) {
                            satisfied = true;
                            break;
                        }
                    } else {
                        new_c.push_back(lit);
                    }
                }
                if (!satisfied && !new_c.empty()) new_clauses.push_back(new_c);
            }
            clauses = move(new_clauses);
        }

        return changed;
    }

    void preprocess() {
        cout << "  [PREPROCESSING] Starting..." << endl << flush;
        int initial = clauses.size();

        bool changed = true;
        int iter = 0;
        while (changed && iter < 10) {
            changed = false;
            iter++;

            if (unit_propagation()) changed = true;
            if (pure_literal_elimination()) changed = true;
        }

        cout << "  [PREPROCESSING] Clauses: " << initial << " → " << clauses.size() << endl << flush;
    }

    vector<vector<int>> get_clauses() { return clauses; }
    vector<int> get_assignment() { return var_value; }
};

// ============================================================================
// MAIN SOLVER
// ============================================================================

class HybridPagerankSATSolver {
private:
    int n_vars;
    vector<vector<int>> clauses;
    vector<vector<int>> lit_to_clauses;
    vector<double> pagerank;
    vector<double> frequency;
    vector<long long> clause_weights;
    vector<double> anchor_history;

    mt19937 rng;
    double pr_weight;
    double mutation_probability;

    // Statistics
    int total_mutations;
    int total_restarts;
    long long total_flips;

    // Tabu
    vector<int> tabu_timestamp;
    vector<int> tabu_tenure;
    int current_tabu_age;

    // Cycle detection
    unordered_set<long long> state_hash;
    vector<int> last_flip_history;
    int cycle_detected_count;

    // Search state
    int stagnation_counter;
    int best_unsat;
    int no_improvement_limit;

    // Implication graph
    vector<vector<int>> imp_graph;
    vector<double> lit_pagerank;

    void build_implication_graph() {
        imp_graph.assign(2 * n_vars + 1, vector<int>());

        for (const auto& clause : clauses) {
            int k = clause.size();
            for (int i = 0; i < k; ++i) {
                int lit_i = clause[i];
                int not_lit_i = (lit_i > 0) ? n_vars + lit_i : abs(lit_i);
                for (int j = 0; j < k; ++j) {
                    if (i == j) continue;
                    int lit_j = clause[j];
                    int lit_j_node = (lit_j > 0) ? lit_j : n_vars + abs(lit_j);
                    imp_graph[not_lit_i].push_back(lit_j_node);
                }
            }
        }

        for (int i = 1; i <= 2 * n_vars; ++i) {
            sort(imp_graph[i].begin(), imp_graph[i].end());
            imp_graph[i].erase(unique(imp_graph[i].begin(), imp_graph[i].end()), imp_graph[i].end());
        }
    }

    void compute_pagerank() {
        lit_pagerank.assign(2 * n_vars + 1, 1.0 / (2 * n_vars));
        vector<double> new_pr(2 * n_vars + 1, 0.0);
        double damping = 0.85;
        double teleport = (1 - damping) / (2 * n_vars);

        for (int iter = 0; iter < 30; ++iter) {
            fill(new_pr.begin(), new_pr.end(), 0.0);
            for (int node = 1; node <= 2 * n_vars; ++node) {
                if (imp_graph[node].empty()) {
                    for (int target = 1; target <= 2 * n_vars; ++target) {
                        new_pr[target] += lit_pagerank[node] / (2 * n_vars);
                    }
                } else {
                    double share = lit_pagerank[node] / imp_graph[node].size();
                    for (int target : imp_graph[node]) {
                        new_pr[target] += share;
                    }
                }
            }
            for (int node = 1; node <= 2 * n_vars; ++node) {
                new_pr[node] = teleport + damping * new_pr[node];
            }
            lit_pagerank.swap(new_pr);
        }

        pagerank.assign(n_vars + 1, 0.0);
        for (int v = 1; v <= n_vars; ++v) {
            pagerank[v] = max(lit_pagerank[v], lit_pagerank[n_vars + v]);
        }
        double max_pr = *max_element(pagerank.begin() + 1, pagerank.end());
        if (max_pr > 0) {
            for (int v = 1; v <= n_vars; ++v) pagerank[v] /= max_pr;
        }
    }

    long long compute_state_hash(const vector<int>& assignment, int unsat_count) {
        long long hash = unsat_count;
        for (int v = 1; v <= n_vars; v += 100) {
            hash = hash * 1000003 + assignment[v];
        }
        return hash;
    }

    void update_clause_weights(const set<int>& unsat) {
        for (int cid : unsat) {
            clause_weights[cid]++;
        }

        static int decay_counter = 0;
        decay_counter++;
        if (decay_counter >= 10000) {
            for (int i = 0; i < (int)clause_weights.size(); ++i) {
                clause_weights[i] = max(1LL, clause_weights[i] / 2);
            }
            decay_counter = 0;
        }
    }

    bool try_exhaustive_last_clause(const set<int>& unsat, vector<int>& assignment) {
        if (unsat.size() != 1) return false;

        int last_clause_id = *unsat.begin();
        const vector<int>& clause = clauses[last_clause_id];

        vector<int> vars_in_clause;
        for (int lit : clause) vars_in_clause.push_back(abs(lit));
        sort(vars_in_clause.begin(), vars_in_clause.end());
        vars_in_clause.erase(unique(vars_in_clause.begin(), vars_in_clause.end()), vars_in_clause.end());

        int k = vars_in_clause.size();
        vector<int> saved(k);
        for (int i = 0; i < k; ++i) saved[i] = assignment[vars_in_clause[i]];

        for (int mask = 0; mask < (1 << k); ++mask) {
            for (int i = 0; i < k; ++i) assignment[vars_in_clause[i]] = (mask >> i) & 1;

            bool all_sat = true;
            for (const auto& c : clauses) {
                bool c_sat = false;
                for (int lit : c) {
                    int v = abs(lit);
                    if ((lit > 0 && assignment[v] == 1) || (lit < 0 && assignment[v] == 0)) {
                        c_sat = true;
                        break;
                    }
                }
                if (!c_sat) { all_sat = false; break; }
            }
            if (all_sat) return true;
        }

        for (int i = 0; i < k; ++i) assignment[vars_in_clause[i]] = saved[i];
        return false;
    }

public:
    HybridPagerankSATSolver(const vector<vector<int>>& cls, int n)
        : clauses(cls), n_vars(n), rng(chrono::steady_clock::now().time_since_epoch().count()) {

        lit_to_clauses.resize(2 * n_vars + 1);
        clause_weights.assign(clauses.size(), 1);

        vector<int> pos_freq(n_vars + 1, 0), neg_freq(n_vars + 1, 0);
        for (int i = 0; i < (int)clauses.size(); ++i) {
            for (int lit : clauses[i]) {
                int v = abs(lit);
                if (lit > 0) pos_freq[v]++;
                else neg_freq[v]++;
                lit_to_clauses[(lit > 0) ? v : n_vars + v].push_back(i);
            }
        }

        frequency.assign(n_vars + 1, 0.0);
        for (int v = 1; v <= n_vars; ++v) {
            double total = pos_freq[v] + neg_freq[v];
            if (total > 0) frequency[v] = (pos_freq[v] - neg_freq[v]) / total;
        }

        build_implication_graph();
        compute_pagerank();

        #if ANCHOR_LOCKING
        anchor_history.assign(n_vars + 1, 0.0);
        #endif

        #if TABU_SEARCH
        tabu_timestamp.assign(n_vars + 1, -1000);
        tabu_tenure.assign(n_vars + 1, 0);
        #endif

        total_mutations = 0;
        total_restarts = 0;
        total_flips = 0;
        stagnation_counter = 0;
        best_unsat = clauses.size();
        no_improvement_limit = 10000;
        cycle_detected_count = 0;
        current_tabu_age = 0;

        pr_weight = 20.0;
        mutation_probability = 0.05;
    }

    void adaptive_mutation(vector<int>& assignment, int current_unsat) {
        total_mutations++;

        double mutation_rate;
        if (current_unsat <= 5) mutation_rate = 0.08;
        else if (current_unsat <= 20) mutation_rate = 0.20;
        else if (current_unsat <= 100) mutation_rate = 0.35;
        else mutation_rate = 0.50;

        vector<pair<double, int>> pressure;
        for (int v = 1; v <= n_vars; ++v) pressure.emplace_back(pagerank[v], v);
        sort(pressure.rbegin(), pressure.rend());

        int mutations = max(1, (int)(n_vars * mutation_rate));
        for (int i = 0; i < min(mutations, n_vars / 5); ++i) {
            int v = pressure[i].second;
            assignment[v] = 1 - assignment[v];
            #if ANCHOR_LOCKING
            anchor_history[v] = 0;
            #endif
        }

        stagnation_counter = 0;
        mutation_probability = min(0.35, mutation_probability * 1.2);
    }

    bool search(vector<int>& assignment, int max_flips = 5000000) {
        vector<int> sat_count(clauses.size(), 0);
        set<int> unsat;

        for (int i = 0; i < (int)clauses.size(); ++i) {
            for (int lit : clauses[i]) {
                int v = abs(lit);
                if ((lit > 0 && assignment[v] == 1) || (lit < 0 && assignment[v] == 0)) {
                    sat_count[i]++;
                }
            }
            if (sat_count[i] == 0) unsat.insert(i);
        }

        long long flip_count = 0;
        long long last_improvement = 0;
        int local_best = unsat.size();

        #if ANCHOR_LOCKING
        fill(anchor_history.begin(), anchor_history.end(), 0.0);
        #endif

        #if TABU_SEARCH
        fill(tabu_timestamp.begin(), tabu_timestamp.end(), -1000);
        fill(tabu_tenure.begin(), tabu_tenure.end(), 0);
        current_tabu_age = 0;
        #endif

        #if CYCLE_DETECTION
        state_hash.clear();
        last_flip_history.clear();
        cycle_detected_count = 0;
        #endif

        for (flip_count = 0; flip_count < max_flips; ++flip_count) {
            if (unsat.empty()) return true;

            int current_unsat = unsat.size();
            if (current_unsat < local_best) {
                local_best = current_unsat;
                last_improvement = flip_count;
                if (current_unsat == 0) return true;
                #if ANCHOR_LOCKING
                stagnation_counter = 0;
                #endif
            }

            #if CLAUSE_WEIGHTING
            if (flip_count % 1000 == 0) {
                update_clause_weights(unsat);
            }
            #endif

            // Select highest weight unsatisfied clause
            int target_clause = -1;
            long long max_weight = -1;
            for (int cid : unsat) {
                if (clause_weights[cid] > max_weight) {
                    max_weight = clause_weights[cid];
                    target_clause = cid;
                }
            }

            const auto& clause = clauses[target_clause];
            vector<int> candidates;
            for (int lit : clause) candidates.push_back(abs(lit));

            int best_var = candidates[0];
            double best_score = -1e18;

            for (int v : candidates) {
                double score = pagerank[v] * 20.0;

                int add_idx = (assignment[v] == 0) ? v : n_vars + v;
                int rem_idx = (assignment[v] == 1) ? v : n_vars + v;

                long long weight_gain = 0;
                for (int cid : lit_to_clauses[add_idx]) {
                    if (sat_count[cid] == 0) weight_gain += clause_weights[cid];
                }
                for (int cid : lit_to_clauses[rem_idx]) {
                    if (sat_count[cid] == 1) weight_gain -= clause_weights[cid];
                }
                score += weight_gain * 15.0;

                #if ANCHOR_LOCKING
                score -= anchor_history[v] * 5.0;
                #endif

                #if TABU_SEARCH
                int age_since_last = current_tabu_age - tabu_timestamp[v];
                if (age_since_last < tabu_tenure[v]) {
                    score -= 500 * (tabu_tenure[v] - age_since_last);
                }
                #endif

                if (score > best_score) {
                    best_score = score;
                    best_var = v;
                }
            }

            // Flip variable
            assignment[best_var] = 1 - assignment[best_var];

            int added_lit = (assignment[best_var] == 1) ? best_var : n_vars + best_var;
            int removed_lit = (assignment[best_var] == 0) ? best_var : n_vars + best_var;

            for (int cid : lit_to_clauses[added_lit]) {
                if (sat_count[cid] == 0 && unsat.find(cid) != unsat.end()) {
                    unsat.erase(cid);
                }
                sat_count[cid]++;
            }

            for (int cid : lit_to_clauses[removed_lit]) {
                sat_count[cid]--;
                if (sat_count[cid] == 0) {
                    unsat.insert(cid);
                }
            }

            #if ANCHOR_LOCKING
            anchor_history[best_var] = anchor_history[best_var] * 0.9 + 1.0;
            #endif

            #if TABU_SEARCH
            tabu_timestamp[best_var] = current_tabu_age;
            tabu_tenure[best_var] = 10 + (rng() % 20);
            current_tabu_age++;
            #endif

            total_flips++;

            #if LAST_CLAUSE
            if (current_unsat == 1) {
                if (try_exhaustive_last_clause(unsat, assignment)) return true;
            }
            #endif

            #if CYCLE_DETECTION
            if (flip_count > 1000 && flip_count % 100 == 0) {
                long long state_key = compute_state_hash(assignment, unsat.size());
                if (state_hash.find(state_key) != state_hash.end()) {
                    cycle_detected_count++;
                    if (cycle_detected_count >= 3) {
                        int force_mutations = max(10, n_vars / 50);
                        for (int i = 0; i < force_mutations; ++i) {
                            int rand_var = (rng() % n_vars) + 1;
                            assignment[rand_var] = 1 - assignment[rand_var];
                            int a_lit = (assignment[rand_var] == 1) ? rand_var : n_vars + rand_var;
                            int r_lit = (assignment[rand_var] == 0) ? rand_var : n_vars + rand_var;
                            for (int cid : lit_to_clauses[a_lit]) {
                                if (sat_count[cid] == 0 && unsat.find(cid) != unsat.end()) unsat.erase(cid);
                                sat_count[cid]++;
                            }
                            for (int cid : lit_to_clauses[r_lit]) {
                                sat_count[cid]--;
                                if (sat_count[cid] == 0) unsat.insert(cid);
                            }
                        }
                        state_hash.clear();
                        cycle_detected_count = 0;
                        last_improvement = flip_count;
                        continue;
                    }
                } else {
                    if ((int)state_hash.size() > 10000) state_hash.clear();
                    state_hash.insert(state_key);
                }
            }
            #endif

            // Stagnation and mutation
            if (flip_count - last_improvement > no_improvement_limit) {
                stagnation_counter++;

                #if ADAPTIVE_MUTATION
                if (stagnation_counter >= 3) {
                    adaptive_mutation(assignment, current_unsat);

                    // Recompute state after mutation
                    fill(sat_count.begin(), sat_count.end(), 0);
                    unsat.clear();
                    for (int i = 0; i < (int)clauses.size(); ++i) {
                        for (int lit : clauses[i]) {
                            int v = abs(lit);
                            if ((lit > 0 && assignment[v] == 1) || (lit < 0 && assignment[v] == 0)) {
                                sat_count[i]++;
                            }
                        }
                        if (sat_count[i] == 0) unsat.insert(i);
                    }
                    last_improvement = flip_count;
                    continue;
                }
                #endif
            } else {
                stagnation_counter = 0;
            }
        }

        return false;
    }

    void solve(string output_path) {
        auto start_time = high_resolution_clock::now();
        bool success = false;
        vector<int> best_assignment;

        for (int restart = 0; restart < 5; ++restart) {
            vector<int> assignment(n_vars + 1, 0);

            #if FREQUENCY_BIAS
            for (int v = 1; v <= n_vars; ++v) {
                if (frequency[v] > 0.2) assignment[v] = 1;
                else if (frequency[v] < -0.2) assignment[v] = 0;
                else assignment[v] = rng() % 2;
            }
            #else
            for (int v = 1; v <= n_vars; ++v) {
                assignment[v] = rng() % 2;
            }
            #endif

            if (search(assignment)) {
                success = true;
                best_assignment = assignment;
                break;
            }
        }

        auto end_time = high_resolution_clock::now();
        double elapsed = duration_cast<duration<double>>(end_time - start_time).count();

        cout << "TIME:" << elapsed << endl;
        cout << "FLIPS:" << total_flips << endl;
        cout << "MUTATIONS:" << total_mutations << endl;

        if (success) {
            cout << "RESULT:SUCCESS" << endl;
            ofstream out(output_path);
            out << "s SATISFIABLE\nv ";
            for (int i = 1; i <= n_vars; ++i) {
                out << (best_assignment[i] ? i : -i) << " ";
                if (i % 15 == 0) out << "\nv ";
            }
            out << "0" << endl;
            out.close();
        } else {
            cout << "RESULT:FAILED" << endl;
            cout << "BEST:" << best_unsat << endl;
        }
    }
};

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input.cnf> <output.solution>" << endl;
        return 1;
    }

    string input_path = argv[1];
    string output_path = argv[2];

    ifstream f(input_path);
    if (!f) {
        cerr << "Cannot open file: " << input_path << endl;
        return 1;
    }

    int n_vars = 0, n_clauses = 0;
    string line;
    vector<vector<int>> clauses;

    while (getline(f, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            stringstream ss(line);
            string tmp;
            ss >> tmp >> tmp >> n_vars >> n_clauses;
        } else {
            stringstream ss(line);
            int lit;
            vector<int> clause;
            while (ss >> lit && lit != 0) clause.push_back(lit);
            if (!clause.empty()) clauses.push_back(clause);
        }
    }
    f.close();

    Preprocessor preprocessor(clauses, n_vars);
    preprocessor.preprocess();
    clauses = preprocessor.get_clauses();

    HybridPagerankSATSolver solver(clauses, n_vars);
    solver.solve(output_path);

    return 0;
}
