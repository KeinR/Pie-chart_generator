package net.keinr.pichart;

import java.io.*;
import java.util.ArrayList;
import java.util.Random;

import net.keinr.util.image.ToFile;
import net.keinr.util.position.*;

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

public class Main extends Application {

    private static final int radius = 50;
    private static final int sideMargin = 10;

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

        int size = numbers.size();

        Vector vector = new Vector(90);
        Pane canvas = new Pane();
        CoordinatePair center = new CoordinatePair(radius+sideMargin, radius+sideMargin), target = new CoordinatePair(radius+sideMargin, sideMargin);
        for (int i = 0; i < size; i++) {
            double degrees = numbers.get(i)/total*360;
            String color = String.format("#%06x", random.nextInt(0xffffff + 1));

            double iterations = degrees*10;
            for (double f = 0; f < iterations; f += 0.1) {
                Line line = new Line(center.x, center.y, target.x, target.y);
                line.setStroke(Color.web(color));
                canvas.getChildren().add(line);
                vector.change(0.01);
                target.x += 1 * vector.getX();
                target.y += 1 * vector.getY();
            }
        }

        double dimensions = radius*2+sideMargin*2;
        ToFile.scene(new Scene(canvas, dimensions, dimensions), "png", "output/output.png");

        Platform.exit();
    }












    public static void main(String[] args) { launch(args); }
}
