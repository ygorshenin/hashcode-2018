import java.io.*;

public class Main {
    public static void main(String[] args) {
        final String[] testNames = {
                "a_example",
                "b_should_be_easy",
                "c_no_hurry",
                "d_metropolis",
                "e_high_bonus",
        };

        for (String testName : testNames) {
            InputStream inputStream;
            try {
                inputStream = new FileInputStream(testName + ".in");
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
            OutputStream outputStream;
            try {
                outputStream = new FileOutputStream(testName + ".out");
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
            FastScanner in = new FastScanner(inputStream);
            PrintWriter out = new PrintWriter(outputStream);
            SelfDrivingRides solver = new SelfDrivingRides();
            solver.greedy(testName, in, out);
            out.close();
        }
    }
}

