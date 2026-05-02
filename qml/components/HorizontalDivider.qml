import QtQuick
import QtQuick.Layouts
import Euterpe

Rectangle {

    // The line appears more prominent at the center due to the solid white (1.0 opacity)
    // and tapers off toward the ends with a gradual transition to transparency.
    // This creates a feathered or "faded" visual effect on both sides.

    gradient: Gradient {
        orientation: Gradient.Horizontal
        GradientStop { position: 0.0; color: "transparent" } // Fully transparent at the left edge
        GradientStop { position: 0.3; color: "#ffffff" } // Fade-in transition towards white
        GradientStop { position: 0.5; color: "#ffffff" } // Solid white center of the line
        GradientStop { position: 0.7; color: "#ffffff" } // Fade-out transition towards transparency
        GradientStop { position: 1.0; color: "transparent" } // Fully transparent at the right edge
    }
}
