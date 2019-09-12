package net.keinr.util.position;

public class CoordinatePair {
    public int x, y;
    public CoordinatePair(int x, int y) {
        this.x = x;
        this.y = y;
    }
    public boolean equals(CoordinatePair other) {
        return (this.x == other.x && this.y == other.y);
    }
}
