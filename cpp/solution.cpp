#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

#include <sys/time.h>
using namespace std;

mt19937 g_engine{42};

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
  double CalcUpperBound() const {
    double energy = 0;
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
  explicit Solution(Problem const& problem)
      : m_problem(&problem),
        m_assignment(m_problem->m_numVehicles),
        m_energies(m_problem->m_numVehicles) {
    InitRandom();
    UpdateEnergy();
  }

  void InitRandom() {
    uniform_int_distribution<int> uid(0, m_problem->m_numVehicles - 1);

    vector<int> rides(m_problem->m_numRides);
    iota(rides.begin(), rides.end(), 0);
    shuffle(rides.begin(), rides.end(), g_engine);

    for (const auto& ride : rides) {
      int const vehicle = uid(g_engine);
      m_assignment[vehicle].push_back(ride);
    }
  }

  double CalcEnergy() const {
    double energy = 0;
    for (int vehicle = 0; vehicle < m_problem->m_numVehicles; ++vehicle)
      energy += CalcEnergy(vehicle);
    return energy;
  }

  double UpdateEnergy() {
    double energy = 0;
    for (int vehicle = 0; vehicle < m_problem->m_numVehicles; ++vehicle)
      energy += UpdateEnergy(vehicle);
    m_energy = energy;
    return energy;
  }

  double UpdateEnergy(int vehicle) {
    assert(vehicle < m_problem->m_numVehicles);
    assert(m_energies.size() == m_problem->m_numVehicles);

    double const energy = CalcEnergy(vehicle);
    m_energies[vehicle] = energy;
    return energy;
  }

  double CalcEnergy(int vehicle) const {
    assert(vehicle < m_problem->m_numVehicles);
    assert(m_assignment.size() == m_problem->m_numVehicles);

    auto const& rides = m_problem->m_rides;

    Cell curr(0, 0);
    int time = 0;
    double totalEnergy = 0;

    for (auto const& r : m_assignment[vehicle]) {
      assert(r < rides.size());

      auto const& ride = rides[r];
      int const arrival = time + Distance(curr, ride.m_source);
      int const start = max(arrival, ride.m_earliestStart);

      double energy = 0;
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
  vector<vector<int>> m_assignment;

  vector<double> m_energies;
  double m_energy = 0;
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

struct Move {
  Move() = default;
  Move(int vfrom,
       int pfrom,
       int vto,
       int pto,
       double delta,
       double oldFromEnergy,
       double oldToEnergy)
      : m_vfrom(vfrom),
        m_pfrom(pfrom),
        m_vto(vto),
        m_pto(pto),
        m_delta(delta),
        m_oldFromEnergy(oldFromEnergy),
        m_oldToEnergy(oldToEnergy),
        m_valid(true) {}

  void Revert(Solution& curr) {
    if (m_vfrom == m_vto) {
      auto& assignment = curr.m_assignment[m_vfrom];
      assert(m_pfrom < assignment.size());
      assert(m_pto < assignment.size());
      swap(assignment[m_pfrom], assignment[m_pto]);
      curr.m_energy -= m_delta;
      curr.m_energies[m_vfrom] = m_oldFromEnergy;
      return;
    }

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

  double m_delta = 0;

  double m_oldFromEnergy = 0;
  double m_oldToEnergy = 0;

  bool m_valid = false;
};

ostream& operator<<(ostream& os, Move const& move) {
  os << move.m_vfrom << " ";
  os << move.m_pfrom << " ";
  os << move.m_vto << " ";
  os << move.m_pto << " ";
  os << move.m_delta << " " << move.m_oldFromEnergy << " " << move.m_oldToEnergy
     << " ";
  os << boolalpha << move.m_valid;
  return os;
}

struct Generator {
  explicit Generator(Problem const& problem)
      : m_problem(problem), m_vehicleUID(0, m_problem.m_numVehicles - 1) {}

  Move Generate(Solution& curr) {
    int const kMaxRetries = m_problem.m_numVehicles;

    auto& assignment = curr.m_assignment;

    int vfrom = -1;
    for (int retry = 0; retry < kMaxRetries; ++retry) {
      int u = m_vehicleUID(g_engine);
      if (!assignment[u].empty()) {
        vfrom = u;
        break;
      }
    }

    if (vfrom < 0)
      return {};

    int const vto = m_vehicleUID(g_engine);
    int const pfrom = m_uid(g_engine) % assignment[vfrom].size();

    int pto = 0;
    if (vto == vfrom)
      pto = m_uid(g_engine) % assignment[vto].size();
    else
      pto = m_uid(g_engine) % (assignment[vto].size() + 1);

    return GenerateMove(curr, vfrom, pfrom, vto, pto);
  }

  Move GenerateMove(Solution& curr, int vfrom, int pfrom, int vto, int pto) {
    if (vfrom == vto) {
      double const oldEnergy = curr.m_energy;
      double const oldFromEnergy = curr.m_energies[vfrom];

      auto& assignment = curr.m_assignment[vfrom];
      assert(pfrom < assignment.size());
      assert(pto < assignment.size());

      swap(assignment[pfrom], assignment[pto]);

      curr.m_energy -= curr.m_energies[vfrom];
      curr.UpdateEnergy(vfrom);
      curr.m_energy += curr.m_energies[vfrom];
      return Move(vfrom, pfrom, vto, pto, curr.m_energy - oldEnergy,
                  oldFromEnergy, oldFromEnergy);
    }

    double const oldEnergy = curr.m_energy;
    double const oldFromEnergy = curr.m_energies[vfrom];
    double const oldToEnergy = curr.m_energies[vto];

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
    return Move(vfrom, pfrom, vto, pto, curr.m_energy - oldEnergy,
                oldFromEnergy, oldToEnergy);
  }

  Problem const& m_problem;
  uniform_int_distribution<int> m_vehicleUID;
  uniform_int_distribution<int> m_uid;
};

struct Solver {
  Solver(Problem const& problem, double timeLimitMs)
      : m_problem(problem), m_timeLimitMs(timeLimitMs) {}

  Solution Solve() {
    Generator generator(m_problem);

    Solution curr(m_problem);

    Solution best = curr;

    double const startTimeMs = GetTimeMs();
    double currTimeMs = startTimeMs;

    double const maxT =
        (m_problem.m_numRows + m_problem.m_numCols + m_problem.m_bonus) * 4;
    double const minT = 0.1;
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

      auto move = generator.Generate(curr);

      if (move.m_delta > 0 || m_urd(g_engine) < exp(move.m_delta / currT)) {
        if (curr.m_energy > best.m_energy)
          best = curr;
      } else {
        move.Revert(curr);
      }
    }

    cerr << "Iterations passed: " << iteration << endl;
    assert(fabs(best.CalcEnergy() - best.m_energy) < 1e-6);

    return best;
  }

  Problem const& m_problem;
  double m_timeLimitMs = 0;
  uniform_real_distribution<double> m_urd;
};

int main() {
  ios_base::sync_with_stdio(false);

  Problem problem;
  cin >> problem;

  cerr << "Num vehicles: " << problem.m_numVehicles << endl;
  cerr << "Num rides: " << problem.m_numRides << endl;

  double const bound = problem.CalcUpperBound();
  cerr << "Bound: " << bound << endl;

  Solver solver(problem, 60 * 1000 /* timeLimitMs */);
  auto const solution = solver.Solve();
  double const energy = solution.CalcEnergy();

  cerr << "Energy: " << energy << endl;
  cerr << "Quality: " << energy / bound << endl;
  cout << solution;
  return 0;
}
