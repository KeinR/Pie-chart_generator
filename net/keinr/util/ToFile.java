package net.keinr.util;

/*
javafx.controls
javafx.swing
*/

import javafx.scene.image.WritableImage;
import javafx.embed.swing.SwingFXUtils;
import javax.imageio.ImageIO;
import java.io.IOException;
import java.io.File;
import javafx.scene.Scene;

public class ToFile {
    public static void scene(Scene scene, String format, String path) {
        WritableImage image = scene.snapshot(null);
        try {
            ImageIO.write(SwingFXUtils.fromFXImage(image, null), format, new File(path));
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }
}
