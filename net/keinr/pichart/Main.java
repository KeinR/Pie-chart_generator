package net.keinr.pichart;

import java.io.*;
import java.util.ArrayList;
import java.util.Random;
import java.lang.Math;

import javafx.application.Application;
import javafx.application.Platform;
import javafx.scene.*;
import javafx.scene.layout.*;
import javafx.stage.Stage;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.control.Label;
import javafx.scene.paint.Color;
import javafx.scene.shape.Line;

import net.keinr.util.*;

public class Main extends Application {

    private static final int radius = 500;
    private static final int sideMargin = 10;

    private static CoordinatePair center = new CoordinatePair(radius+sideMargin, radius+sideMargin), target = new CoordinatePair(radius+sideMargin, sideMargin);

    private static Random random = new Random();

    @Override
    public void start(Stage window) {
        System.out.print("Loading config... ");
        ArrayList<Double> numbers = new ArrayList<Double>();
        double total = 0;
        try (BufferedReader br = new BufferedReader(new FileReader(new File("resources/config.txt")))) {
            String line;
            while ((line = br.readLine()) != null) {
                double result = Double.parseDouble(line);
                total += result;
                numbers.add(result);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        System.out.println("Done!");
        System.out.print("Creating chart... ");

        int size = numbers.size();

        Pane canvas = new Pane();
        for (int i = 0; i < size; i++) {
            double degrees = numbers.get(i)/total*360;
            String color = String.format("#%06x", random.nextInt(0xffffff + 1));

            double degreesChange = 1.0/radius*10;
            for (double f = 0; f < degrees; f += degreesChange) {
                Line line = new Line(center.x, center.y, target.x, target.y);
                line.setStroke(Color.web(color));
                canvas.getChildren().add(line);
                rotate(degreesChange);
            }
        }
        System.out.println("Done!");
        System.out.print("Saving as image... ");

        double dimensions = radius*2+sideMargin*2;
        ToFile.scene(new Scene(canvas, dimensions, dimensions), "png", "output/output.png");

        System.out.println("Done!");
        System.out.print("Exiting program... ");

        Platform.exit();
    }

    private static void rotate(double angle) {
        angle = angle * (Math.PI/180);
        double rotatedX = Math.cos(angle) * (target.x - center.x) - Math.sin(angle) * (target.y-center.y) + center.x;
        double rotatedY = Math.sin(angle) * (target.x - center.x) + Math.cos(angle) * (target.y - center.y) + center.y;
        target.x = rotatedX;
        target.y = rotatedY;
    }












    public static void main(String[] args) { launch(args); }
}
