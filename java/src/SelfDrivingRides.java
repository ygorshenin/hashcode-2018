import java.io.PrintWriter;
import java.util.*;

class SelfDrivingRides {
    int height;
    int width;
    int n; // Number of vehicles.
    int k; // Number of rides.
    int bonus;
    int T; // Number of steps of the simulation.
    int[] sr;
    int[] sc;
    int[] fr;
    int[] fc;
    int[] earliestStart;
    int[] latestFinish;
    int[] travelTime;
    int[] timeRange;
    int[] earliestFinish;
    List<Integer>[] assignment;
    int statsNumBonuses;
    Random random = new Random(0);
    final int infinity = (int) 1e9;

    public int[] solve(String testName, FastScanner in, PrintWriter out) {
        readInput(in);

        assignment = new List[n];
        for (int i = 0; i < n; i++) {
            assignment[i] = new ArrayList<>();
        }

        greedy();

        for (int i = 0; i < n; i++) {
            out.print(assignment[i].size());
            for (int x : assignment[i]) {
                out.print(" " + x);
            }
            out.println();
        }

        return printStats(testName);
    }

    private void greedy() {
        Integer[] rideOrder = new Integer[k];
        for (int i = 0; i < k; i++) {
            rideOrder[i] = i;
        }

        int[] latestStart = new int[k];
        for (int i = 0; i < k; i++) {
            latestStart[i] = latestFinish[i] - travelTime[i];
        }

        int[] sortedTravelTimes = travelTime.clone();
        Arrays.sort(sortedTravelTimes);

        int[] carR = new int[n];
        int[] carC = new int[n];
        int[] carBecomesFree = new int[n];
        int[] timeToReach = new int[n];
        int score = calcScore();

        for (int mode = 0; mode < 2; mode++) {
            for (int step = 0; step < 10; step++) {
                Integer[] oldOrder = rideOrder.clone();
                List<Integer>[] oldAssignment = new List[n];
                for (int i = 0; i < n; i++) {
                    oldAssignment[i] = new ArrayList<>(assignment[i]);
                    assignment[i].clear();
                }
                if (step == 0) {
                    Arrays.sort(rideOrder, (u, v) -> (earliestStart[u] - earliestStart[v]));
                } else if (step == 1) {
                    Arrays.sort(rideOrder, (u, v) -> (latestStart[u] - latestStart[v]));
                } else if (step == 2) {
                    Arrays.sort(rideOrder, (u, v) -> -(travelTime[u] - travelTime[v]));
                } else if (step == 3) {
                    Arrays.sort(rideOrder, (u, v) -> (earliestFinish[u] - earliestFinish[v]));
                } else {
                    for (int i = 0; i < k; i++) {
                        int j = random.nextInt(i + 1);
                        int t = rideOrder[i];
                        rideOrder[i] = rideOrder[j];
                        rideOrder[j] = t;
                    }
                }

                if (mode == 0) {
                    for (int rideIt = 0; rideIt < k; rideIt++) {
                        int id = rideOrder[rideIt];
                        List<Integer> candidates = new ArrayList<>();
                        for (int i = 0; i < n; i++) {
                            if (travelTime[id] > sortedTravelTimes[(int) (k * 0.97)] != i < 70) {
                                continue;
                            }
                            timeToReach[i] = carBecomesFree[i] + dist(carR[i], carC[i], sr[id], sc[id]);
                            if (timeToReach[i] + travelTime[id] <= latestFinish[id]) {
                                candidates.add(i);
                            }
                        }
                        if (candidates.isEmpty()) {
                            continue;
                        }

                        Collections.sort(candidates, (u, v) -> (timeToReach[u] - timeToReach[v]));
                        int i = candidates.get(0);
                        assignment[i].add(id);
                        int t = Math.max(earliestStart[id], carBecomesFree[i] + dist(carR[i], carC[i], sr[id], sc[id]));
                        carBecomesFree[i] = t + travelTime[id];
                        carR[i] = fr[id];
                        carC[i] = fc[id];
                    }
                } else {
                    boolean[] dead = new boolean[k];
                    for (int i = 0; i < n; i++) {
                        int r = 0;
                        int c = 0;
                        int t = 0;
                        while (true) {
                            int best = -1;
                            for (int rideIt = 0; rideIt < k; rideIt++) {
                                int id = rideOrder[rideIt];
                                if (dead[id]) {
                                    continue;
                                }
//                                if (travelTime[id] > sortedTravelTimes[(int) (k * 0.96)] != i < 90) {
//                                    continue;
//                                }
                                int nt = Math.max(earliestStart[id], t + dist(r, c, sr[id], sc[id]));
                                if (nt + travelTime[id] > latestFinish[id]) {
                                    continue;
                                }
                                if (best < 0 || dist(r, c, sr[best], sc[best]) > dist(r, c, sr[id], sc[id])) {
                                    best = id;
                                }
                            }
                            if (best < 0) {
                                break;
                            }
                            int id = best;
                            assignment[i].add(id);
                            dead[id] = true;
                            int nt = Math.max(earliestStart[id], t + dist(r, c, sr[id], sc[id]));
                            t = nt + travelTime[id];
                            if (t > latestFinish[id]) {
                                throw new AssertionError(t);
                            }
                            r = fr[id];
                            c = fc[id];
                        }
                    }
                }

                int nScore = calcScore();
                if (score < nScore) {
                    score = nScore;
                } else {
                    rideOrder = oldOrder;
                    assignment = oldAssignment;
                }
            }
        }


        List<Integer> unassigned = new ArrayList<>();
        {
            boolean[] dead = new boolean[k];
            for (int i = 0; i < n; i++) {
                for (int id : assignment[i]) {
                    dead[id] = true;
                }
            }

            for (int id = 0; id < k; id++) {
                if (!dead[id]) {
                    unassigned.add(id);
                }
            }
        }

        long startTime = System.currentTimeMillis();
        double t = 0;
        int i = 0;
        for (int step = 0;; step++) {
            if ((step & 4095) == 0) {
                t = (System.currentTimeMillis() - startTime) / 300000.0;
            }
            if (t > 1) {
                break;
            }
            ++i;
            if (i == n) {
                i = 0;
            }
            int sc = calcCarScore(i);
            for (int it = 0; it < 20; it++) {
                if (assignment[i].isEmpty()) {
                    continue;
                }
                if (unassigned.isEmpty()) {
                    break;
                }
                int move = random.nextInt(3);
                if (move == 0) {
                    int u = random.nextInt(assignment[i].size());
                    int v = random.nextInt(unassigned.size());
                    int au = assignment[i].get(u);
                    int av = unassigned.get(v);
                    assignment[i].set(u, av);
                    int nsc = calcCarScore(i);
                    if (sc < nsc || sc - nsc < random.nextDouble() * (1 - t) * 1e1) {
                        sc = nsc;
                        unassigned.remove(v);
                        unassigned.add(au);
                    } else {
                        assignment[i].set(u, au);
                    }
                }
                if (move == 1) {
                    int u = random.nextInt(assignment[i].size());
                    int v = random.nextInt(unassigned.size());
                    int av = unassigned.get(v);
                    assignment[i].add(u, av);
                    int nsc = calcCarScore(i);
                    if (sc < nsc || sc - nsc < random.nextDouble() * (1 - t) * 1e1) {
                        sc = nsc;
                        unassigned.remove(v);
                    } else {
                        assignment[i].remove(u);
                    }
                }
                if (move == 2) {
                    int u = random.nextInt(assignment[i].size());
                    int au = assignment[i].get(u);
                    assignment[i].remove(u);
                    int nsc = calcCarScore(i);
                    if (sc < nsc || sc - nsc < random.nextDouble() * (1 - t) * 1e1) {
                        sc = nsc;
                        unassigned.add(au);
                    } else {
                        assignment[i].add(u, au);
                    }
                }
            }
            removeUnreachable(i, unassigned);
        }
    }

    private void removeUnreachable(int i, List<Integer> unassigned) {
        int r = 0;
        int c = 0;
        int t = 0;
        List<Integer> reachable = new ArrayList<>();
        for (int it = 0; it < assignment[i].size(); it++) {
            int id = assignment[i].get(it);
            int nt = Math.max(earliestStart[id], t + dist(r, c, sr[id], sc[id]));
            if (nt + travelTime[id] > latestFinish[id]) {
                unassigned.add(id);
                continue;
            }
            reachable.add(id);
            t = nt + travelTime[id];
            r = fr[id];
            c = fc[id];
        }

        assignment[i] = reachable;
    }

    private int dist(int r1, int c1, int r2, int c2) {
        return Math.abs(r1 - r2) + Math.abs(c1 - c2);
    }

    private int[] printStats(String testName) {
        System.out.println(testName);
        System.out.printf("Bonus = %d\n", bonus);
        int totalRides = 0;
        for (int i = 0; i < n; i++) {
            totalRides += assignment[i].size();
        }
        int score = calcScore();
        int bestScore = calcBestPossibleScore();
        System.out.printf("Score: %d (%.2f%%)\n", score, 100.0 * score / bestScore);
        System.out.printf("Best possible score: %d\n", bestScore);
        System.out.printf("Assigned rides: %d/%d (%.2f%%)\n", totalRides, k, 100.0 * totalRides / k);
        System.out.printf("Recieved bonus: %d/%d (%.2f%%)\n", statsNumBonuses, k, 100.0 * statsNumBonuses / k);
        System.out.println();

//        int totalTravelTime = 0;
//        for (int i = 0; i < k; i++) {
//            totalTravelTime += travelTime[i];
//        }
//        System.out.println(totalTravelTime + " " + (T*n));

        return new int[]{score, bestScore};
    }

    private int calcScore() {
        statsNumBonuses = 0;
        int res = 0;
        for (int i = 0; i < n; i++) {
            res += calcCarScore(i);
        }
        return res;
    }

    private int calcCarScore(int i) {
        int res = 0;
        int r = 0;
        int c = 0;
        int t = 0;
        for (int id : assignment[i]) {
            int nt = Math.max(earliestStart[id], t + dist(r, c, sr[id], sc[id]));
            if (nt + travelTime[id] > latestFinish[id]) {
//                return -infinity;
//                throw new AssertionError(nt);
                continue;
            }
            if (nt == earliestStart[id]) {
                res += bonus;
                ++statsNumBonuses;
            }
            t = nt + travelTime[id];
            r = fr[id];
            c = fc[id];
            res += travelTime[id];
        }
        return res;
    }

    private int calcBestPossibleScore() {
        long res = 0;
        for (int i = 0; i < k; i++) {
            res += travelTime[i] + bonus;
        }
        if (res > Integer.MAX_VALUE) {
            throw new AssertionError();
        }
        return (int)res;
    }

    private void readInput(FastScanner in) {
        height = in.nextInt();
        width = in.nextInt();
        n = in.nextInt();
        k = in.nextInt();
        bonus = in.nextInt();
        T = in.nextInt();

        sr = new int[k];
        sc = new int[k];
        fr = new int[k];
        fc = new int[k];
        earliestStart = new int[k];
        latestFinish = new int[k];
        travelTime = new int[k];
        timeRange = new int[k];
        earliestFinish = new int[k];
        for (int i = 0; i < k; i++) {
            sr[i] = in.nextInt();
            sc[i] = in.nextInt();
            fr[i] = in.nextInt();
            fc[i] = in.nextInt();
            earliestStart[i] = in.nextInt();
            latestFinish[i] = in.nextInt();
            travelTime[i] = dist(sr[i], sc[i], fr[i], fc[i]);
            timeRange[i] = latestFinish[i] - earliestStart[i];
            earliestFinish[i] = earliestStart[i] + travelTime[i];
        }
    }

}
