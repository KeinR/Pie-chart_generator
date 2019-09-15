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
import javafx.scene.text.Font;

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
    private static String title = "";
    private static double titleSize = 15;

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
                try {
                    if (line.charAt(0) == '@') {
                        String[] params = parseCommand(line);
                        if (params[0].equalsIgnoreCase("@radius")) {
                            radius = Double.parseDouble(params[1]);
                        } else if (params[0].equalsIgnoreCase("@margin")) {
                            sideMargin = Double.parseDouble(params[1]);
                        } else if (params[0].equalsIgnoreCase("@outputpath")) {
                            outputPath = params[1];
                        } else if (params[0].equalsIgnoreCase("@title")) {
                            title = params[1];
                        } else if (params[0].equalsIgnoreCase("@titlesize")) {
                            titleSize = Double.parseDouble(params[1]);
                        }
                    } else {
                        String[] params = parseValue(line);
                        double result = params[0].length()>0?Double.parseDouble(params[0]):0;
                        total += result;
                        numbers.add(new Value(result, params[1].length()>1?params[1]:"", params[2].length()>2?params[2]:""));
                    }
                } catch (NumberFormatException e) {
                    System.out.println("Error parsing config @ "+configPath+": parameter on line "+lineNumber+" is not a number");
                    Platform.exit();
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }


        animation.terminate();
        System.out.println(" Done!");
        System.out.print("Creating chart... \r");
        animation = new LoadingAnimation("Creating chart... ", 200);
        new Thread(animation).start();

        final double dimensions = radius*2+sideMargin*2, dimensions_d10 = dimensions/10, dimensions_d20 = dimensions/20;

        Text text_t = new Text(title);
        Font font_t = Font.font("Arial", titleSize);
        text_t.setFont(font_t);
        text_t.applyCss();
        double height_t = text_t.getLayoutBounds().getHeight();
        double width_t = text_t.getLayoutBounds().getWidth();
        Label t = new Label(title);
        t.setLayoutX((dimensions-width_t)/2);
        t.setLayoutY(5);
        t.setStyle("-fx-font-size: "+titleSize+";");
        canvas.getChildren().add(t);

        center = new CoordinatePair(radius+sideMargin, radius+sideMargin+height_t);
        target = new CoordinatePair(radius+sideMargin, sideMargin+height_t);


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

        final double fontSizePre = dimensions_d20/3;
        final double textSize = fontSizePre<10?10:fontSizePre;
        textStyle = "-fx-font-size: "+textSize+";-fx-background-color: transparent;-fx-font-weight: bold;";

        ///

        double y_level = sideMargin, x_level = 0, x_buffer = 0;

        for (int i = 0; i < size; i++) { // Add labels
            Value focus = numbers.get(i);
            String toText = " -> " + focus.name + ",  %" + (Math.round(focus.value/total*10000)/100.0);
            Text text = new Text(toText);
            Font font = Font.font("Arial", textSize);
            text.setFont(font);
            text.applyCss();
            double width = text.getLayoutBounds().getWidth()*1.1;

            // width + color key width + total margin
            double netWidth = dimensions_d20+width+keySpacing;

            if (y_level+dimensions_d20 >= dimensions-sideMargin) {
                y_level = sideMargin;
                x_level = x_buffer;
            }
            if (netWidth+x_level+keySpacing > x_buffer) {
                x_buffer = netWidth+x_level+keySpacing;
            }

            // if (i == 3) {
            //     System.out.println((netWidth+x_level+keySpacing) + " ==? " + x_buffer);
            // }

            Rectangle coloredKey = new Rectangle(dimensions+x_level, y_level+height_t, dimensions_d20, dimensions_d20);
            coloredKey.setFill(Color.web(focus.color));

            Label label = new Label(toText);
            label.setLayoutX(dimensions+x_level+dimensions_d20+keySpacing);
            label.setLayoutY(y_level+height_t+dimensions_d20/4);
            label.setStyle(textStyle);

            canvas.getChildren().addAll(label, coloredKey);

            y_level += dimensions_d20+keySpacing;
        }


        animation.terminate();
        System.out.println(" Done!");
        System.out.print("Saving as image...\r");
        animation = new LoadingAnimation("Saving as image...", 200);
        new Thread(animation).start();

        canvas.setStyle("-fx-background-color: null;");
        Scene scene = new Scene(canvas, dimensions+x_buffer, dimensions+height_t);
        window.initStyle(StageStyle.TRANSPARENT);
        scene.setFill(Color.TRANSPARENT);
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

    private static String[] parseCommand(String line) {
        int size = line.length();
        boolean command = false;
        StringBuilder builder = new StringBuilder();
        String[] send = {"", ""};
        for (int i = 0; i < size; i++) {
            char focus = line.charAt(i);
            if (!command && focus == ' ') {
                command = true;
                send[0] = builder.toString();
                builder = new StringBuilder();
            } else {
                builder.append(Character.toString(focus));
            }
        }
        send[1] = builder.toString();
        return send;
    }

    private static String[] parseValue(String line) {
        String[] send = {"", "", ""};
        int size = line.length();
        StringBuilder builder = new StringBuilder();
        byte stage = 0;
        for (int i = 0; i < size; i++) {
            char focus = line.charAt(i);
            boolean reset = false;

            if (stage == 0 && focus == ' ') {
                reset = true;
                send[0] = builder.toString();
            } else if (stage == 1 && focus == ' ' && i+1 < size && line.charAt(i+1) == '#') {
                reset = true;
                send[1] = builder.toString();
            } else if (focus != '\\' || i+1 >= size || line.charAt(i+1) != '#') {
                builder.append(Character.toString(focus));
            }
            if (reset) {
                stage++;
                builder = new StringBuilder();
            }
        }
        send[stage] = builder.toString();
        return send;
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
