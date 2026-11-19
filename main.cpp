#include<random>
#include<algorithm>
#include<chrono>
#include<numeric>
#include<matplot/matplot.h>

const double EPS = 0.0001;

int E_SIZE = 10000; // smart mutator его не использует, этот параметр используется только для создания исходной популяции
double THRESHOLD = 0.005;
double MUTATE_GEN_PROB = 0.01;
double MUTATE_SPECIES_PROB = 0.3;
int GENERATION_AMOUNT = 1000000;
int POPULATION_SIZE = 20;
double MINX = 0;
double MAXX = 10;
double STEP = 0.1;
double RANGE = std::abs(MAXX - MINX) / STEP + 1;
std::random_device rd;
std::mt19937 gen(rd());

auto range(const double& minx, const double& maxx, const double& step) {
    return std::abs(maxx - minx) / step + 1;
}

auto random(const double& left, const double& right) {
    std::uniform_real_distribution<double> r(left, right - EPS);
    auto rand = [r]() mutable { return r(gen); };
    return rand;
}

auto random_normal(const double& m, const double& s) {
    std::normal_distribution<double> r(m, s);
    auto rand = [r]() mutable { return r(gen); };
    return rand;
}

// auto target_function = [](const auto& x) { return static_cast<double>(x * x * x * sin(x)); };
// auto target_function = [](const auto& x) { return static_cast<double>(2 * x * x); };
// auto target_function = [](const auto& x) { return std::log(2 * x); };
auto target_function = [](const auto& x) { return static_cast<double>(std::pow(x, 32)); };
// auto target_function = [](const auto& x) { return std::sin(3 * x); };

void show_function() {
    using namespace matplot;
    std::vector<double> x(RANGE);
    std::vector<double> y(RANGE);
    for (int i = 0; i < RANGE; i++) {
        x[i] = i / 10.0;
        auto result = target_function(x[i]);
        y[i] = std::isnan(result) ? 0 : result;
    }
    plot(x, y);
    show();
}

class Species {
protected:
    // [0, 0.1, 0.2, ..., 9.9, 10]
    std::vector<double> _dots;

public:
    Species(): _dots(RANGE) {
        auto rand = random(-E_SIZE, E_SIZE);
        for (int i = 0; i < RANGE; i++) {
            // [0.1, 10]
            _dots[i] = rand();
        }
    }

    Species(const std::vector<double>& dots): _dots(std::move(dots)) {}

    decltype(auto) get_dots() const {
        const auto& ref_dots = _dots;
        return ref_dots;
    }

    auto fitness() const {
        double s = 0;
        for (int i = 0; i < RANGE; i++) {
            auto result = target_function(i * STEP);
            if (!std::isinf(result) && !std::isnan(result)) {
                s += std::abs(_dots[i] - result);
            }
        }
        return s;
    }

    virtual std::vector<Species> crossover(const Species& other) const {
        auto split_dot = static_cast<int>(random(0, RANGE)());
        std::vector<double> dd1;
        std::vector<double> dd2;
        dd1.insert(dd1.begin(), _dots.begin(), _dots.begin() + split_dot);
        dd1.insert(dd1.begin() + split_dot, other._dots.begin() + split_dot, other._dots.end());
        dd2.insert(dd2.begin(), other._dots.begin(), other._dots.begin() + split_dot);
        dd2.insert(dd2.begin() + split_dot, _dots.begin() + split_dot, _dots.end());
        Species d1(dd1);
        Species d2(dd2);
        return std::vector<Species>{d1, d2};
    }

    virtual void mutate() {
        auto rand = random(0, 1);
        auto rand_E = random(-E_SIZE, E_SIZE);
        for (int i = 0; i < _dots.size(); i++) {
            if (rand() < MUTATE_GEN_PROB) {
                _dots[i] = rand_E();
            }
        }
    }

    void print() const {
        for (int i = 0; i < RANGE; i++) {
            printf("%f %f\n", i * STEP, _dots[i]);
        }
    }

    void plot_show(const std::string& plot_style = "") const {
        using namespace matplot;
        std::vector<double> x;
        std::vector<double> y;
        for (int i = 0; i < RANGE; i++) {
            x.push_back(i * STEP);
            y.push_back(_dots[i]);
        }
        plot(x, y, plot_style);
        show();
    }
};

class Species_TASK: public Species {
public:
    Species_TASK(): Species() {}
    Species_TASK(const std::vector<double>& dots): Species(std::move(dots)) {}

    auto crossover(const Species_TASK& other) {
        // auto u = random(0, 1)();
        // std::vector<double> dd1;
        // std::vector<double> dd2;
        // for (int i = 0; i < 100; i++) {
        //     auto y1i = u * (this->get_dots()[i]) + (1 - u) * other.get_dots()[i];
        //     auto y2i = u * other.get_dots()[i] + (1 - u) * this->get_dots()[i];
        //     dd1.push_back(y1i);
        //     dd2.push_back(y2i);
        // }
        // Species_TASK d1(dd1);
        // Species_TASK d2(dd2);
        // return std::vector<Species_TASK>{d1, d2};
        auto split_dot = static_cast<int>(random(0, RANGE)());
        std::vector<double> dd1;
        std::vector<double> dd2;
        dd1.insert(dd1.begin(), _dots.begin(), _dots.begin() + split_dot);
        dd1.insert(dd1.begin() + split_dot, other._dots.begin() + split_dot, other._dots.end());
        dd2.insert(dd2.begin(), other._dots.begin(), other._dots.begin() + split_dot);
        dd2.insert(dd2.begin() + split_dot, _dots.begin() + split_dot, _dots.end());
        Species_TASK d1(dd1);
        Species_TASK d2(dd2);
        return std::vector<Species_TASK>{d1, d2};
    }

    void mutate() {
        auto rand = random(0, 1);
        auto rand_E = random_normal(0, 1);
        auto species_fitness = this->fitness();
        int a = 1, b = 0;
        for (int i = 0; i < _dots.size(); i++) {
            if (rand() < MUTATE_GEN_PROB) {
                _dots[i] = _dots[i] + rand_E() * (a * species_fitness + b);
            }
            // if ((i + 1) % _dots.size() == 0)
            //     std::cout << "rand_E() " << (rand_E() * std::sqrt(a * species_fitness + b)) << '\n';
        };
    }

};

template<typename S>
class GenAlg {
private:
    std::vector<std::pair<int, double>> __history;

    void __add_to_history(int current_generation) {
        auto average_fitness = std::accumulate(
            _species.begin(),
            _species.end(),
            0.0,
            [](const auto& sum, const auto& species) { return sum + species.fitness(); }) / POPULATION_SIZE;
        __history.push_back(std::make_pair(current_generation, average_fitness));
    }

protected:
    std::vector<S> _species;

    void _generate_population() {
        _species.resize(0);
        for (int i = 0; i < POPULATION_SIZE; i++) {
            S sp;
            _species.push_back(sp);
        }
    }

    void _breeding() {
        auto rand = random(0, POPULATION_SIZE);
        auto crossover_amount = POPULATION_SIZE / 2;
        // std::vector<S> children;
        for (int i = 0; i < crossover_amount; i++) {
            // панмиксия
            auto pi1 = static_cast<int>(rand());
            auto pi2 = static_cast<int>(rand());
            if (pi2 == pi1) {
                pi2 = (pi2 + 1) % POPULATION_SIZE;
            }
            const auto& descendants = _species[pi1].crossover(_species[pi2]);
            _species.insert(_species.end(), descendants.begin(), descendants.end());
        }
    }

    void _mutation() {
        auto rand = random(0, 1);
        for (int i = POPULATION_SIZE; i < POPULATION_SIZE * 2; i++) {
            if (rand() < MUTATE_SPECIES_PROB) {
                _species[i].mutate();
            }
        }
    }

    virtual void _new_population() {
        std::vector<S> species;
        double sum_fitness = std::accumulate(
            _species.begin(),
            _species.end(),
            0,
            [](const auto& sum, const auto& species) { return sum + species.fitness(); }
        );
        int c = 0;
        auto rand = random(0.0, sum_fitness);
        while (c < POPULATION_SIZE) {
            double pick = rand();
            double current_sum = 0;
            for (int i = 0; i < POPULATION_SIZE * 2; i++) {
                current_sum += _species[i].fitness();
                if (current_sum >= pick) {
                    species.push_back(_species[i]);
                    break;
                }
            }
            c++;
        }
        _species = std::move(species);
    }


    // все _species находятся в отсортированном виде
    auto _is_finished() {
        if (_species[0].fitness() <= THRESHOLD) return 0;
        return -1;
    }

public:
    GenAlg() {}
    GenAlg(double mutate_prob, int generation_amount): _mutate_prob_percent(mutate_prob), _generation_amount(generation_amount) {}

    auto get_generations_amount() const {
        return __history.size();
    }

    auto get_species(const int& index) const {
        return _species[index];
    }

    auto execute() {
        _generate_population();
        int i = 0;
        while (i < GENERATION_AMOUNT) {
            __add_to_history(i);
            if (i % 1000 == 0) printf("%i %f\n", i, _species[0].fitness());
            _breeding();
            _mutation();
            _new_population();
            auto finished = _is_finished();
            if (finished > -1) {
                return finished;
            }
            i++;
        }
        return -1;
    }

    void show_history(const int& from, const int& to, const std::string& plot_style = "") {
        using namespace matplot;
        std::vector<int> generations;
        std::vector<double> average_fitness;
        for (int i = from; i < std::min<int>(to, __history.size()); i++) {
            auto e = __history[i];
            generations.push_back(e.first);
            average_fitness.push_back(e.second);
        }
        plot(generations, average_fitness, plot_style);
        show();
    }
};

template<typename S>
class GenAlg_TASK: public GenAlg<S> {
protected:
    void _new_population() {
        std::vector<std::pair<int, double>> fitnesses;
        for (int i = 0; i < POPULATION_SIZE * 2; i++) {
            fitnesses.push_back(std::make_pair(i, _species[i].fitness()));
        }
        std::sort(fitnesses.begin(), fitnesses.end(), [](const auto& x, const auto& y) { return x.second < y.second; });
        std::vector<S> new_species;
        for (int i = 0; i < POPULATION_SIZE; i++) {
            auto ind = fitnesses[i].first;
            new_species.push_back(_species[ind]);
        }
        _species = std::move(new_species);
    }
};

void init() {
    E_SIZE = 10;
    THRESHOLD = 0.005;
    MUTATE_GEN_PROB = 0.01;
    MUTATE_SPECIES_PROB = 1;
    GENERATION_AMOUNT = 1000000;
    POPULATION_SIZE = 20;
    MINX = 0;
    MAXX = 10;
    STEP = 0.1;
    RANGE = range(MAXX, MINX, STEP);
}

int main() {
    init();
    GenAlg_TASK<Species_TASK> ga;
    auto start = std::chrono::steady_clock::now();
    auto indx = ga.execute();
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> seconds = end - start;
    printf("Fitness %f; generations %d\n", ga.get_species(0).fitness(), ga.get_generations_amount());
    printf("Time executing %f\n", seconds);
    ga.show_history(0, GENERATION_AMOUNT);
    ga.get_species(0).plot_show();
    ga.get_species(0).print();
    matplot::hold(matplot::on);
    show_function();
}
