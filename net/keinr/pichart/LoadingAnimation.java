package net.keinr.pichart;

import java.lang.InterruptedException;

class LoadingAnimation implements Runnable {

    private static final String[] animationKey = {"|", "/", "-", "\\"};

    boolean alive;
    long intervalMillis;
    String baseString;
    byte animationPhase;

    LoadingAnimation(String baseString, long intervalMillis) {
        this.intervalMillis = intervalMillis;
        this.baseString = baseString;
        alive = true;
        animationPhase = 0;
    }

    synchronized void terminate() {
        alive = false;
        System.out.print(baseString);
    }

    @Override
    public void run() {
        while (alive) {
            System.out.print(baseString+" "+animationKey[animationPhase]+"\r");
            if (++animationPhase > 3) animationPhase = 0;
            try {
                Thread.sleep(intervalMillis);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}
