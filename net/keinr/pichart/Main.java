package net.keinr.pichart;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.FileReader;
import java.io.File;

import java.util.ArrayList;
import java.util.Random;

import java.lang.Math;

import javafx.application.Application;
import javafx.application.Platform;

import javafx.stage.Stage;
import javafx.stage.StageStyle;

import javafx.scene.Scene;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.scene.control.Label;
import javafx.scene.paint.Color;
import javafx.scene.shape.Line;
import javafx.scene.text.Text;

import net.keinr.util.*;

public class Main extends Application {

    private static final int radius = 500;
    private static final int sideMargin = 10;

    private static CoordinatePair center = new CoordinatePair(radius+sideMargin, radius+sideMargin), target = new CoordinatePair(radius+sideMargin, sideMargin);

    private static Random random = new Random();

    @Override
    public void start(Stage window) {
        System.out.print("Loading config... ");

        ArrayList<Value> numbers = new ArrayList<Value>();
        double total = 0;
        try (BufferedReader br = new BufferedReader(new FileReader(new File("resources/config.txt")))) {
            String line;
            while ((line = br.readLine()) != null) {
                String[] params = line.split("\\s");
                double result = params.length>0?Double.parseDouble(params[0]):0;
                total += result;

                numbers.add(new Value(result, params.length>1?params[1]:"", params.length>2?params[2]:""));
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        System.out.println("Done!");
        System.out.print("Creating chart... ");

        int size = numbers.size();

        Pane canvas = new Pane();
        for (int i = 0; i < size; i++) { // Draw chart
            Value focus = numbers.get(i);
            double degrees = focus.value/total*360;

            double degreesChange = 1.0/radius*10;
            for (double f = 0; f < degrees; f += degreesChange) {
                Line line = new Line(center.x, center.y, target.x, target.y);
                line.setStroke(Color.web(focus.color));
                canvas.getChildren().add(line);
                rotate(degreesChange);
            }
        }

        double spaceRemaining = dimensions, y_level = dimensions+10, x_buffer = 0;
        int onCurrentLevel = 0;
        for (int i = 0; i < size; i++) { // Add labels
            Value focus = numbers.get(i);
            String toText = focus.color + " = " + focus.name + ",  %" + (focus.value/total);
            final Text text = new Text(toText);
            text.applyCss();
            final double width = text.getLayoutBounds().getWidth();

            if (width >= dimensions + x_buffer) {
                if ()
            }
        }

        System.out.println("Done!");
        System.out.print("Saving as image... ");

        double dimensions = radius*2+sideMargin*2;

        Scene scene = new Scene(canvas, dimensions+x_buffer, y_level+10);
        window.initStyle(StageStyle.TRANSPARENT);
        scene.setFill(Color.TRANSPARENT);
        ToFile.scene(scene, "png", "output/output.png");

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


    class Value {
        double value;
        String name, color;
        Value(double value, String name, String color) {
            this.value = value;
            this.name = name;
            this.color = color==""?String.format("#%06x", random.nextInt(0xffffff + 1)):color;
        }
    }









    public static void main(String[] args) { launch(args); }
}
