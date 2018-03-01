import java.io.*;
import java.util.Locale;

public class Main {
    public static void main(String[] args) {
        Locale.setDefault(Locale.UK);
        final String[] testNames = {
//                "a_example",
//                "b_should_be_easy",
                "c_no_hurry",
//                "d_metropolis",
//                "e_high_bonus",
        };

        int totalScore = 0;
        int bestPossibleScore = 0;
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
            int[] scores = solver.solve(testName, in, out);
            totalScore += scores[0];
            bestPossibleScore += scores[1];
            out.close();
        }
//        System.out.println(bestPossibleScore);
        System.out.printf("Total score: %.2fM (%.2f%%)\n", totalScore * 1e-6, 100.0 * totalScore / bestPossibleScore);
    }
}

