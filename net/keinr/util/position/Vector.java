package net.keinr.util.position;

/*
Y-Axis is flipped by default for easy coordinate manipulation.
Vector starts at 0 degrees.
Disclamer: Due to doubles and their fuckery, only ~99.999% accurate
*/

public class Vector {
    private static final double dpv = 4/360.0;
    private static boolean invertedY = true;
    private double x, y, vector_raw;
    public Vector() {
        x = 0;
        y = 1;
        vector_raw = 0;
    }
    public Vector(double initialDegrees) {
        x = 0;
        y = 1;
        vector_raw = 0;
        change(initialDegrees);
    }
    public double getX() { return x; }
    public double getY() { return y * (invertedY?-1:1); }
    public static void toggleInvertedYAxis(boolean toggle) { invertedY = toggle; }
    public void change(double degrees) {
        vector_raw += degrees * dpv;

        while (vector_raw > 4) {
            vector_raw -= 4;
        }
        double real = vector_raw;

        if (vector_raw >= 3) {
            real -= 3;
            y = real;
            x = -1 * (1 - real);
        } else if (vector_raw >= 2) {
            real -= 2;
            y = -1 * (1 - real);
            x = -1 * real;
        } else if (vector_raw >= 1) {
            real -= 1;
            y = -1 * real;
            x = 1 - real;
        } else if (vector_raw >= 0) {
            y = 1 - real;
            x = real;
        }

        if (y == -0.0) {
            y = 0.0;
        }
        if (x == -0.0) {
            x = 0.0;
        }
    }
}
