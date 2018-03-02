#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

#include <gflags/gflags.h>
#include <sys/time.h>
using namespace std;

DEFINE_int32(pre_limit_sec, 10, "Preliminary time limit in seconds");
DEFINE_int32(post_limit_sec, 10, "Post time limit in seconds");

struct Context {
  Context() = default;
  Context(int seed) : m_engine(seed) {}

  mt19937 m_engine;
};

double GetTimeMs() {
  timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

struct Cell {
  Cell() = default;
  Cell(int row, int col) : m_row(row), m_col(col) {}

  int m_row = 0;
  int m_col = 0;
};

int Distance(Cell const& lhs, Cell const& rhs) {
  return abs(lhs.m_row - rhs.m_row) + abs(lhs.m_col - rhs.m_col);
}

istream& operator>>(istream& is, Cell& cell) {
  is >> cell.m_row >> cell.m_col;
  return is;
}

struct Ride {
  int Length() const { return Distance(m_source, m_target); }

  Cell m_source;
  Cell m_target;

  int m_earliestStart = 0;
  int m_latestStart = 0;
};

istream& operator>>(istream& is, Ride& ride) {
  is >> ride.m_source;
  is >> ride.m_target;
  is >> ride.m_earliestStart;

  int latestFinish;
  is >> latestFinish;
  ride.m_latestStart = latestFinish - ride.Length();
  return is;
}

struct Problem {
  int64_t CalcUpperBound() const {
    int64_t energy = 0;
    for (auto const& ride : m_rides)
      energy += ride.Length() + m_bonus;
    return energy;
  }

  int m_numRows = 0;
  int m_numCols = 0;
  int m_numVehicles = 0;
  int m_numRides = 0;
  int m_bonus = 0;
  int m_numSteps = 0;
  vector<Ride> m_rides;
};

istream& operator>>(istream& is, Problem& problem) {
  is >> problem.m_numRows;
  is >> problem.m_numCols;
  is >> problem.m_numVehicles;
  is >> problem.m_numRides;
  is >> problem.m_bonus;
  is >> problem.m_numSteps;

  problem.m_rides.clear();
  for (int i = 0; i < problem.m_numRides; ++i) {
    Ride ride;
    is >> ride;
    problem.m_rides.push_back(ride);
  }
  return is;
}

struct Solution {
  Solution() = default;

  Solution(Problem const& problem, Context& context)
      : m_problem(&problem),
        m_context(&context),
        m_assignment(m_problem->m_numVehicles),
        m_energies(m_problem->m_numVehicles) {
    InitRandom();
    UpdateEnergy();
  }

  void InitRandom() {
    uniform_int_distribution<int> uid(0, m_problem->m_numVehicles - 1);

    vector<int> rides(m_problem->m_numRides);
    iota(rides.begin(), rides.end(), 0);
    shuffle(rides.begin(), rides.end(), m_context->m_engine);

    for (const auto& ride : rides) {
      int const vehicle = uid(m_context->m_engine);
      m_assignment[vehicle].push_back(ride);
    }
  }

  int64_t CalcEnergy() const {
    int64_t energy = 0;
    for (int vehicle = 0; vehicle < m_problem->m_numVehicles; ++vehicle)
      energy += CalcEnergy(vehicle);
    return energy;
  }

  int64_t UpdateEnergy() {
    int64_t energy = 0;
    for (int vehicle = 0; vehicle < m_problem->m_numVehicles; ++vehicle)
      energy += UpdateEnergy(vehicle);
    m_energy = energy;
    return energy;
  }

  int64_t UpdateEnergy(int vehicle) {
    assert(vehicle < m_problem->m_numVehicles);
    assert(m_energies.size() == m_problem->m_numVehicles);

    return m_energies[vehicle] = CalcEnergy(vehicle);
  }

  int64_t CalcEnergy(int vehicle) const {
    assert(vehicle < m_problem->m_numVehicles);
    assert(m_assignment.size() == m_problem->m_numVehicles);

    auto const& rides = m_problem->m_rides;

    Cell curr(0, 0);
    int time = 0;
    int64_t totalEnergy = 0;

    for (auto const& r : m_assignment[vehicle]) {
      assert(r < rides.size());

      auto const& ride = rides[r];
      int const arrival = time + Distance(curr, ride.m_source);
      int const start = max(arrival, ride.m_earliestStart);

      int64_t energy = 0;
      if (start == ride.m_earliestStart)
        energy += m_problem->m_bonus;
      if (start <= ride.m_latestStart)
        energy += ride.Length();

      int const finish = start + ride.Length();

      curr = ride.m_target;
      time = finish;
      totalEnergy += energy;
    }

    return totalEnergy;
  }

  Problem const* m_problem;
  Context* m_context;
  vector<vector<int>> m_assignment;

  vector<int64_t> m_energies;
  int64_t m_energy = 0;
};

ostream& operator<<(ostream& os, Solution const& solution) {
  for (auto const& assignment : solution.m_assignment) {
    os << assignment.size();
    for (auto const r : assignment)
      os << " " << r;
    os << endl;
  }
  return os;
}

struct Base {
  virtual void Revert(Solution& curr) = 0;

  int64_t m_delta = 0;
  bool m_valid = false;
};

struct Invalid : public Base {
  Invalid() { m_valid = false; }

  void Revert(Solution& curr) override {}
};

struct Shift : public Base {
  Shift() { m_valid = true; }

  void Revert(Solution& curr) override {
    if (m_from == m_to)
      return;

    auto& assignment = curr.m_assignment[m_vehicle];
    assert(curr.m_energies.size() == curr.m_assignment.size());
    assert(m_from < assignment.size());
    assert(m_to < assignment.size());

    int const ride = assignment[m_to];

    assignment.erase(assignment.begin() + m_to);
    assignment.insert(assignment.begin() + m_from, ride);

    curr.m_energy -= m_delta;
    curr.m_energies[m_vehicle] = m_energy;
  }

  int m_vehicle = 0;
  int m_from = 0;
  int m_to = 0;
  int64_t m_energy = 0;
};

struct Move : public Base {
  Move() { m_valid = true; }

  void Revert(Solution& curr) override {
    assert(m_vfrom != m_vto);

    auto& afrom = curr.m_assignment[m_vfrom];
    auto& ato = curr.m_assignment[m_vto];

    assert(m_pfrom <= afrom.size());
    assert(m_pto < ato.size());

    int const ride = ato[m_pto];
    ato.erase(ato.begin() + m_pto);
    afrom.insert(afrom.begin() + m_pfrom, ride);

    curr.m_energies[m_vfrom] = m_oldFromEnergy;
    curr.m_energies[m_vto] = m_oldToEnergy;
    curr.m_energy -= m_delta;
  }

  int m_vfrom = 0;
  int m_pfrom = 0;

  int m_vto = 0;
  int m_pto = 0;

  int64_t m_oldFromEnergy = 0;
  int64_t m_oldToEnergy = 0;
};

ostream& operator<<(ostream& os, Move const& move) {
  os << move.m_vfrom << " ";
  os << move.m_pfrom << " ";
  os << move.m_vto << " ";
  os << move.m_pto << " ";
  os << move.m_delta << " " << move.m_oldFromEnergy << " " << move.m_oldToEnergy << " ";
  os << boolalpha << move.m_valid;
  return os;
}

template <bool ShiftsOnly>
struct Generator {
  Generator(Problem const& problem, Context& context)
      : m_problem(problem), m_context(context), m_vehicleUID(0, m_problem.m_numVehicles - 1) {}

  Base& Generate(Solution& curr) {
    int const kMaxRetries = m_problem.m_numVehicles;

    auto& assignment = curr.m_assignment;

    int vfrom = -1;
    for (int retry = 0; retry < kMaxRetries; ++retry) {
      int u = m_vehicleUID(m_context.m_engine);
      if (!assignment[u].empty()) {
        vfrom = u;
        break;
      }
    }

    if (vfrom < 0)
      return m_invalid;

    int const pfrom = m_uid(m_context.m_engine) % assignment[vfrom].size();

    if (ShiftsOnly) {
      int const vto = vfrom;
      int const pto = m_uid(m_context.m_engine) % assignment[vto].size();
      return GenerateShift(curr, vfrom, pfrom, pto);
    }

    int const vto = m_vehicleUID(m_context.m_engine);

    if (vfrom == vto) {
      int const pto = m_uid(m_context.m_engine) % assignment[vto].size();
      return GenerateShift(curr, vfrom, pfrom, pto);
    }

    int const pto = m_uid(m_context.m_engine) % (assignment[vto].size() + 1);
    return GenerateMove(curr, vfrom, pfrom, vto, pto);
  }

  Shift& GenerateShift(Solution& curr, int vehicle, int from, int to) {
    auto const oldEnergy = curr.m_energy;
    auto const oldFromEnergy = curr.m_energies[vehicle];

    auto& assignment = curr.m_assignment[vehicle];
    assert(from < assignment.size());
    assert(to < assignment.size());

    if (from != to) {
      int const ride = assignment[from];
      assignment.erase(assignment.begin() + from);
      assignment.insert(assignment.begin() + to, ride);
      curr.m_energy -= curr.m_energies[vehicle];
      curr.UpdateEnergy(vehicle);
      curr.m_energy += curr.m_energies[vehicle];
    }

    m_shift.m_vehicle = vehicle;
    m_shift.m_from = from;
    m_shift.m_to = to;
    m_shift.m_delta = curr.m_energy - oldEnergy;
    m_shift.m_energy = oldFromEnergy;
    return m_shift;
  }

  Move& GenerateMove(Solution& curr, int vfrom, int pfrom, int vto, int pto) {
    assert(vfrom != vto);

    auto const oldEnergy = curr.m_energy;
    auto const oldFromEnergy = curr.m_energies[vfrom];
    auto const oldToEnergy = curr.m_energies[vto];

    curr.m_energy -= oldFromEnergy + oldToEnergy;

    auto& afrom = curr.m_assignment[vfrom];
    auto& ato = curr.m_assignment[vto];

    assert(pfrom < afrom.size());
    int const ride = afrom[pfrom];
    afrom.erase(afrom.begin() + pfrom);

    assert(pto <= ato.size());
    ato.insert(ato.begin() + pto, ride);

    curr.UpdateEnergy(vfrom);
    curr.UpdateEnergy(vto);

    curr.m_energy += curr.m_energies[vfrom] + curr.m_energies[vto];

    m_move.m_vfrom = vfrom;
    m_move.m_vto = vto;
    m_move.m_pfrom = pfrom;
    m_move.m_pto = pto;
    m_move.m_delta = curr.m_energy - oldEnergy;
    m_move.m_oldFromEnergy = oldFromEnergy;
    m_move.m_oldToEnergy = oldToEnergy;
    return m_move;
  }

  Problem const& m_problem;
  Context& m_context;
  uniform_int_distribution<int> m_vehicleUID;
  uniform_int_distribution<int> m_uid;

  Invalid m_invalid;
  Shift m_shift;
  Move m_move;
};

struct Solver {
  Solver(Problem const& problem, Context& context, double timeLimitMs)
      : m_problem(problem), m_context(context), m_timeLimitMs(timeLimitMs) {}

  template <bool ShiftsOnly>
  Solution Solve() {
    Solution curr(m_problem, m_context);
    return Solve<ShiftsOnly>(curr);
  }

  template <bool ShiftsOnly>
  Solution Solve(Solution curr) {
    Generator<ShiftsOnly> generator(m_problem, m_context);

    Solution best = curr;

    double const startTimeMs = GetTimeMs();
    double currTimeMs = startTimeMs;

    double const maxT = ShiftsOnly ? 10 : (m_problem.m_numRows + m_problem.m_numCols + m_problem.m_bonus) * 8;
    double const minT = 0.01;
    double currT = maxT;

    uint64_t iteration = 0;
    for (;; ++iteration) {
      if ((iteration & 0xFFF) == 0) {
        currTimeMs = GetTimeMs();
        if (currTimeMs >= startTimeMs + m_timeLimitMs)
          break;
        double const progress = (currTimeMs - startTimeMs) / m_timeLimitMs;
        currT = maxT * pow(minT / maxT, progress);
      }

      auto& move = generator.Generate(curr);

      assert(curr.m_energy == curr.CalcEnergy());

      if (!move.m_valid)
        continue;

      if (move.m_delta > 0 || m_urd(m_context.m_engine) < exp(static_cast<double>(move.m_delta) / currT)) {
        if (curr.m_energy > best.m_energy)
          best = curr;
      } else {
        move.Revert(curr);
      }
    }

    cerr << "Iterations passed: " << iteration << endl;
    assert(best.CalcEnergy() == best.m_energy);

    return best;
  }

  Problem const& m_problem;
  Context& m_context;
  double m_timeLimitMs = 0;
  uniform_real_distribution<double> m_urd;
};

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  ios_base::sync_with_stdio(false);

  Problem problem;
  cin >> problem;

  cerr << "Num vehicles: " << problem.m_numVehicles << endl;
  cerr << "Num rides: " << problem.m_numRides << endl;

  int64_t const bound = problem.CalcUpperBound();
  cerr << "Bound: " << bound << endl;

  double const preLimitMs = FLAGS_pre_limit_sec * 1000.0;
  double const postLimitMs = FLAGS_post_limit_sec * 1000.0;

  Context context{42};

  Solver preSolver(problem, context, preLimitMs);
  auto const preSolution = preSolver.Solve<false>();
  cerr << "Pre-solution: " << preSolution.m_energy << endl;

  Solver postSolver(problem, context, postLimitMs);
  auto const postSolution = postSolver.Solve<true>(preSolution);

  cerr << "Energy: " << postSolution.m_energy << endl;
  cerr << "Quality: " << static_cast<double>(postSolution.m_energy) / bound << endl;
  cout << postSolution;
  return 0;
}
