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
    List<Integer>[] assignment;

    public void greedy(String testName, FastScanner in, PrintWriter out) {
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

        printStats(testName);
    }

    private void greedy() {
        Integer[] rideOrder = new Integer[k];
        for (int i = 0; i < k; i++) {
            rideOrder[i] = i;
        }
        Arrays.sort(rideOrder, (u, v) -> (earliestStart[u] - earliestStart[v]));

        int[] carR = new int[n];
        int[] carC = new int[n];
        int[] carBecomesFree = new int[n];
        for (int rideIt = 0; rideIt < k; rideIt++) {
            int id = rideOrder[rideIt];
            int best = -1;
            for (int i = 0; i < n; i++) {
                if (carBecomesFree[i] + dist(carR[i], carC[i], sr[id], sc[id]) + travelTime[id] <= latestFinish[id]
                        && (best < 0 || carBecomesFree[best] > carBecomesFree[i])) {
                    best = i;
                }
            }
            if (best < 0) {
                continue;
            }
            int i = best;
            assignment[i].add(id);
            int t = Math.max(earliestStart[id], carBecomesFree[i] + dist(carR[i], carC[i], sr[id], sc[id]));
//            System.out.println(carBecomesFree[i] + " " + (t + travelTime[id]));
            carBecomesFree[i] = t + travelTime[id];
            carR[i] = fr[id];
            carC[i] = fc[id];
        }
    }

    private int dist(int r1, int c1, int r2, int c2) {
        return Math.abs(r1 - r2) + Math.abs(c1 - c2);
    }

    private void printStats(String testName) {
        System.out.println(testName);
        int totalRides = 0;
        for (int i = 0; i < n; i++) {
            totalRides += assignment[i].size();
        }
        int score = calcScore();
        int bestScore = calcBestPossibleScore();
        System.out.printf("Score: %d (%.2f%%)\n", score, 100.0 * score / bestScore);
        System.out.printf("Best possible score: %d\n", bestScore);
        System.out.printf("Assigned rides: %d/%d\n", totalRides, k);
        System.out.println();
    }

    private int calcScore() {
        int res = 0;
        for (int i = 0; i < n; i++) {
            int r = 0;
            int c = 0;
            int t = 0;
            for (int id : assignment[i]) {
                int nt = Math.max(earliestStart[id], t + dist(r, c, sr[id], sc[id]));
                if (nt == earliestStart[id]) {
                    res += bonus;
                }
                if (nt + travelTime[id] > latestFinish[id]) {
                    throw new AssertionError();
                }
                t = nt + travelTime[id];
                r = fr[id];
                c = fc[id];
                res += travelTime[id];
            }
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
        for (int i = 0; i < k; i++) {
            sr[i] = in.nextInt();
            sc[i] = in.nextInt();
            fr[i] = in.nextInt();
            fc[i] = in.nextInt();
            earliestStart[i] = in.nextInt();
            latestFinish[i] = in.nextInt();
            travelTime[i] = dist(sr[i], sc[i], fr[i], fc[i]);
        }
    }

}
