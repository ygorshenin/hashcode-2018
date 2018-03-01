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
    Set<Integer>[] rides;

    public void solve(int testNumber, FastScanner in, PrintWriter out) {
        readInput(in);

        rides = new Set[n];
        for (int i = 0; i < n; i++) {
            rides[i] = new TreeSet<>();
        }


        Ride[] allRides = new Ride[k];
        for (int i = 0; i < k; i++) {
            allRides[i] = new Ride();
            allRides[i].id = i;
            allRides[i].offset = -1;
        }
        for (int i = 0; i < n; i++) {
            rides[i].add(i);
        }

        for (int i = 0; i < n; i++) {
            out.print(rides[i].size());
            for (int x : rides[i]) {
                out.print(" " + x);
            }
            out.println();
        }
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
        for (int i = 0; i < k; i++) {
            sr[i] = in.nextInt();
            sc[i] = in.nextInt();
            fr[i] = in.nextInt();
            fc[i] = in.nextInt();
            earliestStart[i] = in.nextInt();
            latestFinish[i] = in.nextInt();
        }
    }

    class Ride implements Comparable<Ride> {
        int id; // Global id.
        int offset; // In its car list.

        public int compareTo(Ride o) {
            return id - o.id;
        }
    }

}
