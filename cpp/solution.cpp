#include <iostream>
#include <vector>
using namespace std;

struct Cell {
  Cell() = default;
  Cell(int row, int col) : m_row(row), m_col(col) {}

  int m_row = 0;
  int m_col = 0;
};

istream& operator>>(istream& is, Cell& cell) {
  is >> cell.m_row >> cell.m_col;
  return is;
}

struct Ride {
  Cell m_source;
  Cell m_target;

  int m_earliestStart = 0;
  int m_latestFinish = 0;
};

istream& operator>>(istream& is, Ride& ride) {
  is >> ride.m_source;
  is >> ride.m_target;
  is >> ride.m_earliestStart;
  is >> ride.m_latestFinish;
  return is;
}

struct Problem {
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

int main() {
  ios_base::sync_with_stdio(false);

  Problem problem;
  cin >> problem;
  return 0;
}
