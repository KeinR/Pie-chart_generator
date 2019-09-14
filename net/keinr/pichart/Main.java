package net.keinr.pichart;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.FileReader;
import java.io.File;

import java.util.ArrayList;
import java.util.Random;

import java.lang.Math;
import java.lang.NumberFormatException;

import javafx.application.Application;
import javafx.application.Platform;

import javafx.stage.Stage;
import javafx.stage.StageStyle;

import javafx.scene.Scene;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.scene.control.Label;
import javafx.scene.shape.Line;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Text;

import net.keinr.util.*;

/*
* I am really satisfied with how organized the import statements are.
* I mean like god, that's gotta' be the best part about this project.
* Forget the code below, the real shit's above.
*/

public class Main extends Application {

    private static final String configPath = "resources/config.txt";

    private static final double keySpacing = 5;

    private static double radius = 500;
    private static double sideMargin = 10;
    private static CoordinatePair center, target;
    private static Random random = new Random();
    private static String outputPath = "output/output.png";
    private static Pane canvas = new Pane();
    private static String textStyle = "-fx-font-size: 12;";

    @Override
    public void start(Stage window) {
        System.out.print("Loading config... \r");
        LoadingAnimation animation = new LoadingAnimation("Loading config... ", 200);
        new Thread(animation).start();

        ArrayList<Value> numbers = new ArrayList<Value>();
        double total = 0;
        try (BufferedReader br = new BufferedReader(new FileReader(new File(configPath)))) {
            String line;
            int lineNumber = 0;
            while ((line = br.readLine()) != null) {
                lineNumber++;
                if (line.length() == 0) continue;
                String[] params = line.split("\\s");

                if (line.charAt(0) == '@') {
                    if (params.length < 2) continue;
                    try {
                        if (params[0].equalsIgnoreCase("@radius")) {
                            radius = Double.parseDouble(params[1]);
                        } else if (params[0].equalsIgnoreCase("@margin")) {
                            sideMargin = Double.parseDouble(params[1]);
                        } else if (params[0].equalsIgnoreCase("@outputPath")) {
                            outputPath = params[1];
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("Error parsing config "+configPath+": second parameter on line "+lineNumber+" is not an integer or a double.");
                        Platform.exit();
                    }
                } else {
                    double result = params.length>0?Double.parseDouble(params[0]):0;
                    total += result;
                    numbers.add(new Value(result, params.length>1?params[1]:"", params.length>2?params[2]:""));
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        center = new CoordinatePair(radius+sideMargin, radius+sideMargin);
        target = new CoordinatePair(radius+sideMargin, sideMargin);


        animation.terminate();
        System.out.println(" Done!");
        System.out.print("Creating chart... \r");
        animation = new LoadingAnimation("Creating chart... ", 200);
        new Thread(animation).start();

        int size = numbers.size();

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

        final double dimensions = radius*2+sideMargin*2, dimensions_d10 = dimensions/10, dimensions_d20 = dimensions/20;

        // System.out.println(dimensions_d20);

        final double fontSizePre = dimensions_d20/3;
        textStyle = "-fx-font-size: "+(fontSizePre<10?10:fontSizePre)+";-fx-background-color: transparent;";

        ///

        double spaceRemaining = dimensions, y_level = dimensions, x_buffer = 0, currentBuffer = 0;
        int onCurrentLevel = 0;

        for (int i = 0; i < size; i++) { // Add labels
            Value focus = numbers.get(i);
            String toText = " -> " + focus.name + ",  %" + (Math.round(focus.value/total*10000)/100.0);
            Text text = new Text(toText);
            text.setStyle(textStyle);
            text.applyCss();
            double width = text.getLayoutBounds().getWidth();

            // width + color key width + total margin
            double netWidth = dimensions_d10+width+keySpacing;
            if (netWidth+currentBuffer >= dimensions + x_buffer) {
                if (currentBuffer == 0) {
                    x_buffer += (netWidth+currentBuffer)-(dimensions+x_buffer);
                    addSet(currentBuffer+sideMargin, y_level, dimensions_d20, focus.color, toText);
                    y_level += dimensions_d20;
                } else {
                    currentBuffer = 0;
                    y_level += dimensions_d20;
                    addSet(currentBuffer+sideMargin, y_level, dimensions_d20, focus.color, toText);
                    currentBuffer += netWidth;
                }
            } else {
                addSet(currentBuffer+sideMargin, y_level, dimensions_d20, focus.color, toText);
                currentBuffer += netWidth;
            }
        }

        ///

        animation.terminate();
        System.out.println(" Done!");
        System.out.print("Saving as image...\r");
        animation = new LoadingAnimation("Saving as image...", 200);
        new Thread(animation).start();

        canvas.setStyle("-fx-background-color: null;");
        Scene scene = new Scene(canvas, dimensions+x_buffer, y_level+dimensions_d20);
        window.initStyle(StageStyle.TRANSPARENT);
        scene.setFill(Color.TRANSPARENT);
        // scene.getStylesheets().add("resources/style.css");
        ToFile.scene(scene, "png", outputPath);

        animation.terminate();
        System.out.println(" Done!");
        int leng = outputPath.length();
        System.out.println("/-------------------" + "-".repeat(leng) + "\\\n" +
                        "| Pichart saved to " + outputPath + " |\n" +
                        "\\-------------------" + "-".repeat(leng) + "/"
                        );
        System.out.print("->Exiting program...");

        Platform.exit();
    }

    private static void rotate(double angle) {
        angle = angle * (Math.PI/180);
        double rotatedX = Math.cos(angle) * (target.x - center.x) - Math.sin(angle) * (target.y - center.y) + center.x;
        double rotatedY = Math.sin(angle) * (target.x - center.x) + Math.cos(angle) * (target.y - center.y) + center.y;
        target.x = rotatedX;
        target.y = rotatedY;
    }

    private static void addSet(double x, double y_level, double dimensions_d20, String color, String text) {
        Rectangle coloredKey = new Rectangle(x, y_level, dimensions_d20, dimensions_d20);
        coloredKey.setFill(Color.web(color));

        Label label = new Label(text);
        label.setLayoutX(x+dimensions_d20+keySpacing);
        label.setLayoutY(y_level);
        label.setStyle(textStyle);
        // label.getStyleClass().add("keyText");

        canvas.getChildren().addAll(label, coloredKey);
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
