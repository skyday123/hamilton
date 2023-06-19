#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <set>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <memory>
#include <sys/time.h>

class Graph {
    struct Cycle {
        int id;
        std::unordered_set<int> edges;

        Cycle(int id_) : id(id_) {}

        void add(int edge) {
            assert(edges.count(edge) == 0);
            edges.insert(edge);
        }

        bool has(int edge) {
            return edges.count(edge) == 1;
        }

        int size() {
            return edges.size();
        }

        void merge(Cycle* source) {
            assert(source != nullptr);
            edges.merge(std::move(source->edges)); // requires C++17
            delete source;
        }

        void remove_edge(int edge) {
            auto it = edges.find(edge);
            assert(it != edges.end());
            edges.erase(it);
        }

        int get_random_edge() {
            int index = rand() % edges.size();
            auto it = edges.begin();
            std::advance(it, index);
            return *it;
        }

        int get_next_edge(int edge) {
            auto it = edges.find(edge);
            assert(it != edges.end());
            std::advance(it, 1);
            if (it == edges.end())
                it = edges.begin();
            return *it;
        }

        void show() {
            printf("[Cycle %d] %ld: ", id, edges.size());
            for (const int edge : edges) {
                printf("%d, ", edge);
            }
            printf("\n");
        }
    };

    int n;
    int cycle_num;
    int valid_num;
    Cycle **cycles, **h, **v;

    Cycle* find_edge(int edge) {
        if (is_h(edge)) {
            return h[edge];
        } else {
            edge -= n*(n-1);
            return v[edge];
        }
    }

    void edit_edge(int edge, Cycle *cycle) {
        if (is_h(edge)) {
            h[edge] = cycle;
        } else {
            edge -= n*(n-1);
            v[edge] = cycle;
        }
    }

    bool is_h(int edge) {
        assert(edge >= 0 && edge < 2*n*(n-1));
        return edge < n * (n - 1);
    }

    bool id2coord(int edge, int *i, int *j) {
        assert(edge >= 0 && edge < 2*n*(n-1));
        if (edge < n*(n-1)) {
            *i = edge / (n-1);
            *j = edge % (n-1);     
            return true;
        } else {
            edge -= n*(n-1);
            *i = edge / n;
            *j = edge % n;
            return false;
        }
    }

    int coord2id(int i, int j, bool is_h) {
        if (is_h) {
            return i * (n-1) + j;
        } else {
            return n*(n-1) + i * n + j;
        }
    }

public:
    Graph() = delete;

    Graph(int n_) : n(n_) {
        assert(n > 0 && n % 2 == 0);
        cycle_num = valid_num = (n/2)*(n/2);

        cycles = new Cycle* [cycle_num];
        for (int i=0; i<cycle_num; i++) {
            cycles[i] = new Cycle(i);
        }

        // n*(n-1)
        h = new Cycle* [n*(n-1)];
        // (n-1)*n
        v = new Cycle* [n*(n-1)];

        // init horizontal edges
        for (int i=0; i<n; i++) {
            int cycle_idx = i / 2 * (n / 2);
            for (int j=0; j<n-1; j++) {
                int edge = i * (n - 1) + j;
                if (j % 2 == 0) {
                    cycles[cycle_idx]->add(edge);
                    edit_edge(edge, cycles[cycle_idx]);
                } else {
                    edit_edge(edge, nullptr);
                    cycle_idx++;
                }
            }
        }

        // init vertical edges
        int cycle_idx = 0;
        for (int i=0; i<n-1; i++) {
            for (int j=0; j<n; j++) {
                int edge = n*(n-1) + i*n+j;
                if (i % 2 == 0) {
                    cycles[cycle_idx]->add(edge);
                    edit_edge(edge, cycles[cycle_idx]);
                    if (j % 2 == 1) cycle_idx++;
                } else {
                    edit_edge(edge, nullptr);
                }
            }
        }
    }

    ~Graph() {
        for (int i=0; i<cycle_num; i++) {
            if (cycles[i])
                delete cycles[i];
        }
        delete [] cycles;
        delete [] h;
        delete [] v;
    }

    void step() {
        assert(valid_num > 1);
        // randomly pick a cycle
        int random_cycle_idx = rand() % cycle_num;
        while (cycles[random_cycle_idx] == nullptr) {
            random_cycle_idx = (random_cycle_idx==cycle_num-1) ? 0 : random_cycle_idx+1;
        }
        Cycle *c1 = cycles[random_cycle_idx];
        Cycle *c2;

        int c1_edge = c1->get_random_edge();
        int c2_edge;
        
        while (true) { 
            int i, j;
            if (id2coord(c1_edge, &i, &j)) {
                // h
                // top
                int i_t = i - 1;
                if (i_t >= 0) {
                    c2_edge = coord2id(i_t, j, true);
                    if (find_edge(c2_edge) && !c1->has(c2_edge)) break;
                }
                // bottom
                int i_b = i + 1;
                if (i_b < n) {
                    c2_edge = coord2id(i_b, j, true);
                    if (find_edge(c2_edge) && !c1->has(c2_edge)) break;
                }
            } else {
                // v
                // left
                int j_l = j - 1;
                if (j_l >= 0) {
                    c2_edge = coord2id(i, j_l, false);
                    if (find_edge(c2_edge) && !c1->has(c2_edge)) break;
                }
                // right
                int j_r = j + 1;
                if (j_r < n) {
                    c2_edge = coord2id(i, j_r, false);
                    if (find_edge(c2_edge) && !c1->has(c2_edge)) break;
                }
            }

            c1_edge = c1->get_next_edge(c1_edge);
        }

        c2 = find_edge(c2_edge);

        int new_edge_1, new_edge_2;
        if (is_h(c1_edge)) {
            // h
            int c1_i = c1_edge / (n-1);
            int c2_i = c2_edge / (n-1);
            int j = c1_edge % (n-1);
            int v_i = std::min(c1_i, c2_i);

            new_edge_1 = n*(n-1) + v_i * n + j;
            new_edge_2 = new_edge_1 + 1;
        } else {
            // v
            int c1_j = (c1_edge - n*(n-1)) % n;
            int c2_j = (c2_edge - n*(n-1)) % n;
            int i = (c1_edge - n*(n-1)) / n;
            int h_j = std::min(c1_j, c2_j);

            new_edge_1 = i * (n-1) + h_j;
            new_edge_2 = (i+1) * (n-1) + h_j;
        }

        assert(find_edge(new_edge_1) == nullptr);
        assert(find_edge(new_edge_2) == nullptr);

        c1->remove_edge(c1_edge);
        c2->remove_edge(c2_edge);
        edit_edge(c1_edge, nullptr);
        edit_edge(c2_edge, nullptr);

        // merge two cycles
        // merge small to large
        Cycle *c_small, *c_large;
        if (c1->size() > c2->size()) {
            c_small = c2;
            c_large = c1;
        } else {
            c_small = c1;
            c_large = c2;
        }
        for (int edge : c_small->edges) {
            edit_edge(edge, c_large);
        }
        cycles[c_small->id] = nullptr;
        c_large->merge(c_small);
        c_large->add(new_edge_1);
        c_large->add(new_edge_2);
        edit_edge(new_edge_1, c_large);
        edit_edge(new_edge_2, c_large);

        valid_num--;
    }

    void optimize(int steps=-1) {
        if (steps == -1)
            steps = valid_num - 1;
        assert(steps > 0 && steps <= valid_num-1);
        for (int s=0; s<steps; s++) {
            step();
        }
    }

    void save(const char* output) {
        FILE* f = fopen(output, "w");
        fprintf(f, "%d\n", n);
        for (int i=0; i<n; i++) {
            for (int j=0; j<n-1; j++) {
                int edge = i*(n-1)+j;
                char ch = find_edge(edge) ? '1' : '0';
                putc(ch, f);
            }
            putc('\n', f);
        }
        for (int i=0; i<n-1; i++) {
            for (int j=0; j<n; j++) {
                int edge = n*(n-1)+i*n+j;
                char ch = find_edge(edge) ? '1' : '0';
                putc(ch, f);
            }
            putc('\n', f);
        }
        fclose(f);
        printf("Save to %s\n", output);
    }

    void show() {
        printf("Valid/All: %d/%d\n", valid_num, cycle_num);
        for (int i=0; i<cycle_num; i++) {
            if (cycles[i])
                cycles[i]->show();
            else
                printf("[Cycle %d] null\n", i);
        }
    }
};

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("usage:\nhamilton N OUTPUT SEED\n");
        return -1;
    }

    int n = atoi(argv[1]);
    assert(n % 2 == 0);
    char* output = argv[2];
    int seed = atoi(argv[3]);
    
    srand(seed);
    timeval start_t, end_t;
    gettimeofday(&start_t, NULL);

    Graph graph(n);
    graph.optimize();

    gettimeofday(&end_t, NULL);

    //graph.show();

    graph.save(output);

    double time_ms = (1000000*(end_t.tv_sec-start_t.tv_sec) + end_t.tv_usec - start_t.tv_usec) / 1000.0;
    printf("N = %d, time = %.2f ms\n", n, time_ms);

    return 0;
}
