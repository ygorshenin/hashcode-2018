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
    Set<Integer>[] assignment;

    public void greedy(int testNumber, FastScanner in, PrintWriter out) {
        readInput(in);

        assignment = new Set[n];
        for (int i = 0; i < n; i++) {
            assignment[i] = new TreeSet<>();
        }

        greedy();

        for (int i = 0; i < n; i++) {
            out.print(assignment[i].size());
            for (int x : assignment[i]) {
                out.print(" " + x);
            }
            out.println();
        }

        System.out.println(T);
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
                if (carBecomesFree[i] + dist(sr[id], sc[id], carR[i], carC[i]) <= earliestStart[id]) {
                    best = i;
                }
            }
            if (best < 0) {
                continue;
            }
            assignment[best].add(id);
            int t = earliestStart[id];
            carBecomesFree[best] = t + travelTime[id];
            carR[best] = fr[id];
            carC[best] = fc[id];
        }
    }

    private int dist(int r1, int c1, int r2, int c2) {
        return Math.abs(r1 - r2) + Math.abs(c1 - c2);
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
