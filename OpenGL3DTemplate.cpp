#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <glut.h>
#include <SFML/Audio.hpp>
#include <iostream>


#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
bool wallsAreYellow = true;
int lastMouseX, lastMouseY;
bool isLeftButtonPressed = false;
float cameraAngleX = 0.0f, cameraAngleY = 0.0f;
bool gameRunning = true;  // To track if the game is still running
bool finishLineHit = false;  // To track if the finish line was hit
int gameTimeRemaining = 40;  // Timer for 40 seconds
bool finishLineVisible = true;
bool gameOver = false; // Indicates if the game is over (won or lost)
bool gameWon = false;  // Differentiates between winning and losing
bool animationActive = false; // Toggle for animations
float egyptianFlagAngle = 0.0f; // Rotation angle for the Egyptian flag
float olympicFlagScale = 1.0f;  // Scale factor for the Olympic flag
bool olympicFlagScalingUp = true; // Direction of scaling
int podiumColorIndex = 0;
int dumbellsColorIndex1 = 0;
int dumbellsColorIndex2 = 0;// Index for the current color
const float podiumColors[3][3] = {
	{1.0f, 0.0f, 0.0f}, // Red
	{0.0f, 1.0f, 0.0f}, // Green
	{0.0f, 0.0f, 1.0f}  // Blue
};
const float dumbellsColors1[3][3] = {
	{1.0f, 1.0f, 0.0f}, // Yellow
	{0.0f, 1.0f, 1.0f}, // Cyan
	{1.0f, 0.0f, 1.0f}  // Magenta
};

const float dumbellsColors2[3][3] = {
	{0.5f, 0.5f, 0.5f}, // Gray
	{1.0f, 0.5f, 0.0f}, // Orange
	{0.0f, 0.0f, 0.0f}  // Black
};
float torchStandXOffset = 0.0f; // Current X offset for the torch stand
bool torchStandMovingRight = true; // Direction of the translation
std::string currentView = ""; // Initialize an empty string for the current view
bool polesAreGreen = true; // Tracks the current color of the poles


sf::SoundBuffer backgroundMusicBuffer;
sf::SoundBuffer jumpSoundBuffer;
sf::SoundBuffer collectibleSoundBuffer;
sf::SoundBuffer collideSoundBuffer;
sf::SoundBuffer gameLostSoundBuffer;
sf::SoundBuffer gameWonSoundBuffer;
sf::SoundBuffer powerup0SoundBuffer;
sf::SoundBuffer powerup1SoundBuffer;

sf::Sound backgroundMusic;
sf::Sound jumpSound;
sf::Sound collectibleSound;
sf::Sound collideSound;
sf::Sound gameLostSound;
sf::Sound gameWonSound;
sf::Sound powerup0Sound;
sf::Sound powerup1Sound;


class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

Camera camera;

Vector3f playerPosition(5.5f, 0.0f, 0.0f);
float playerRotation = -90.0f;
void toggleWallColor(int value) {
	wallsAreYellow = !wallsAreYellow;
	glutPostRedisplay();
	glutTimerFunc(2000, toggleWallColor, 0);
}

void updateAnimations(int value) {
	if (animationActive) {
		// Rotate the Egyptian flag
		egyptianFlagAngle += 2.0f;
		if (egyptianFlagAngle >= 360.0f) {
			egyptianFlagAngle -= 360.0f; // Keep the angle within 0-360 degrees
		}

		// Scale the Olympic flag
		if (olympicFlagScalingUp) {
			olympicFlagScale += 0.01f;
			if (olympicFlagScale >= 1.3f) {
				olympicFlagScalingUp = false;
			}
		}
		else {
			olympicFlagScale -= 0.01f;
			if (olympicFlagScale <= 1.0f) {
				olympicFlagScalingUp = true;
			}
		}

		// Translate the torch stand
		if (torchStandMovingRight) {
			torchStandXOffset += 0.02f; // Move to the right
			if (torchStandXOffset >= 1.0f) {
				torchStandMovingRight = false; // Switch direction
			}
		}
		else {
			torchStandXOffset -= 0.02f; // Move to the left
			if (torchStandXOffset <= -1.0f) {
				torchStandMovingRight = true; // Switch direction
			}
		}

		// Change podium color every 1 second
		static int colorChangeCounter = 0;
		colorChangeCounter++;
		if (colorChangeCounter >= 62) { // ~62 iterations at 16ms per iteration equals 1 second
			colorChangeCounter = 0;
			podiumColorIndex = (podiumColorIndex + 1) % 3; // Cycle through 0, 1, 2
			dumbellsColorIndex1 = (dumbellsColorIndex1 + 1) % 3; // Cycle through 0, 1, 2
			dumbellsColorIndex2 = (dumbellsColorIndex2 + 1) % 3; // Cycle through 0, 1, 2
		}

		glutPostRedisplay(); // Request a redraw
		glutTimerFunc(16, updateAnimations, 0); // 60 FPS
	}
}

void drawEgyptianFlag() {
	// Draw the static base
	glPushMatrix();
	glColor3f(0.4f, 0.2f, 0.0f); // Brown for the base
	glTranslated(0.0, 0.25, -4.0); // Position the base (height adjusted)
	glScaled(0.8, 0.5, 0.8);       // Increase height of the base
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the static flagpole
	glPushMatrix();
	glColor3f(0.8f, 0.8f, 0.8f); // Light grey for the pole
	glTranslated(0.0, 1.0, -4.0); // Adjust position to start above the taller base
	glScaled(0.05, 1.0, 0.05);     // Scale to make it a tall, thin pole
	glutSolidCube(1.0);
	glPopMatrix();

	// Rotate the flag sections around the pole
	glPushMatrix();
	glTranslated(-5.0, 1.5, -4.0); // Move to the top of the adjusted pole

	if (animationActive) {
		glRotatef(egyptianFlagAngle, 0.0f, 1.0f, 0.0f); // Rotate around Y-axis
	}

	// Draw the black part of the flag (bottom section)
	glColor3f(0.0f, 0.0f, 0.0f); // Black color
	glPushMatrix();
	glTranslated(5.35, -0.2, 0.0); // Offset flag part from the pole
	glScaled(0.6, 0.2, 0.05);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the white part of the flag (middle section)
	glColor3f(1.0f, 1.0f, 1.0f); // White color
	glPushMatrix();
	glTranslated(5.35, 0.0, 0.0); // Offset flag part from the pole
	glScaled(0.6, 0.2, 0.05);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the red part of the flag (top section)
	glColor3f(1.0f, 0.0f, 0.0f); // Red color
	glPushMatrix();
	glTranslated(5.35, 0.2, 0.0); // Offset flag part from the pole
	glScaled(0.6, 0.2, 0.05);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix(); // End rotation transformation
}

void setView(const std::string& view) {
	if (view == "top") {
		camera.eye = Vector3f(0.0f, 8.0f, 0.0f); // Decreased y-value for a closer top-down view
		camera.center = Vector3f(0.0f, 0.0f, 0.0f); // Look at the center of the scene
		camera.up = Vector3f(0.0f, 0.0f, -1.0f); // Set the up direction to negative z-axis
	}
	else if (view == "side") {
		camera.eye = Vector3f(10.0f, 3.0f, 0.0f); // Decreased y-value for a lower side view
		camera.center = Vector3f(0.0f, 0.0f, 0.0f); // Look at the center of the scene
		camera.up = Vector3f(0.0f, 1.0f, 0.0f); // Set the up direction to positive y-axis
	}
	else if (view == "front") {
		camera.eye = Vector3f(0.0f, 3.0f, 10.0f); // Front view remains the same
		camera.center = Vector3f(0.0f, 0.0f, 0.0f); // Look at the center of the scene
		camera.up = Vector3f(0.0f, 1.0f, 0.0f); // Set the up direction to positive y-axis
	}
	glutPostRedisplay(); // Update the display to reflect the new camera position
}

void drawFloor() {
	// Set the color to green for the floor
	glColor3f(0.0f, 1.0f, 0.0f);

	// Draw the floor
	glPushMatrix();
	glTranslated(0.0, 0.0, 0.0); // Center the floor
	glScaled(16.0, 0.01, 10.0); // Adjust size to match the area between the walls
	glutSolidCube(1.0);
	glPopMatrix();
}

void drawWalls() {
	if (wallsAreYellow) {
		glColor3f(1.0f, 1.0f, 0.0f); // Yellow
	}
	else {
		glColor3f(1.0f, 0.0f, 0.0f); // Red
	}

	// Draw wall 1 (back wall)
	glPushMatrix();
	glTranslated(0.0, 0.25, -5.0); // Adjusted to align with new height
	glScaled(16.0, 0.5, 0.1); // Half height
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 0.75, -5.0); // Slightly above the first cube
	glScaled(16.0, 0.5, 0.1); // Half height
	glutSolidCube(1.0);
	glPopMatrix();

	// Interior top-center cube for wall 1
	glPushMatrix();
	glTranslated(0.0, 1.0, -4.95); // Inside and at the top center
	glutSolidCube(0.5);
	glPopMatrix();

	// Draw wall 2 (left wall)
	glPushMatrix();
	glTranslated(-8.0, 0.25, 0.0); // Adjusted to align with new height
	glScaled(0.1, 0.5, 10.0); // Half height
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslated(-8.0, 0.75, 0.0); // Slightly above the first cube
	glScaled(0.1, 0.5, 10.0); // Half height
	glutSolidCube(1.0);
	glPopMatrix();

	// Interior top-center cube for wall 2
	glPushMatrix();
	glTranslated(-7.95, 1.0, 0.0); // Inside and at the top center
	glutSolidCube(0.5);
	glPopMatrix();

	// Draw wall 3 (right wall)
	glPushMatrix();
	glTranslated(8.0, 0.25, 0.0); // Adjusted to align with new height
	glScaled(0.1, 0.5, 10.0); // Half height
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslated(8.0, 0.75, 0.0); // Slightly above the first cube
	glScaled(0.1, 0.5, 10.0); // Half height
	glutSolidCube(1.0);
	glPopMatrix();

	// Interior top-center cube for wall 3
	glPushMatrix();
	glTranslated(7.95, 1.0, 0.0); // Inside and at the top center
	glutSolidCube(0.5);
	glPopMatrix();
}

void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, 640.0 / 480.0, 0.001, 100);  // Increased FOV to 90 for even more zoomed-out effect

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

// Function to toggle pole colors
void togglePoleColors(int value) {
	polesAreGreen = !polesAreGreen; // Toggle the color flag
	glutPostRedisplay();            // Request a redraw to reflect the color change
	glutTimerFunc(1000, togglePoleColors, 0); // Call this function again after 1 second
}

void drawFinishLine() {
	if (!finishLineVisible) return; // Skip drawing if the finish line is not visible

	glPushMatrix();

	// Adjust the overall position (move left and lower the height)
	glTranslatef(-2.0, -0.1, -3.0); // Move left by -1.0 along x-axis, lower by -0.2 along y-axis
	glRotatef(90, 0.0, 1.0, 0.0);  // Rotate around the y-axis by 90 degrees

	// Set color for poles based on toggle
	if (polesAreGreen) {
		glColor3f(0.0f, 1.0f, 0.0f); // Green
	}
	else {
		glColor3f(0.0f, 0.0f, 0.0f); // Black
	}

	// Draw the left pole
	glPushMatrix();
	glTranslated(-6.0, 0.3, -3.0); // Position the left pole (lower height)
	glScaled(0.1, 0.5, 0.1);      // Reduce pole height (y-axis scaling decreased from 1.0 to 0.8)
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the right pole
	glPushMatrix();
	glTranslated(0.0, 0.3, -3.0); // Position the right pole (lower height)
	glScaled(0.1, 0.5, 0.1);      // Reduce pole height (y-axis scaling decreased from 1.0 to 0.8)
	glutSolidCube(1.0);
	glPopMatrix();

	if (polesAreGreen) {
		glColor3f(0.0f, 0.0f, 0.0f); // Black
	}
	else {

		glColor3f(0.0f, 1.0f, 0.0f); // Green

	}
	// Draw the horizontal connecting cylinder
	glPushMatrix();
	glTranslated(-6.0, 0.5, -3.0); // Adjust position to match the poles
	glRotatef(90, 0.0, 1.0, 0.0); // Rotate to make it horizontal along the x-axis
	GLUquadric* quad = gluNewQuadric();
	gluCylinder(quad, 0.05, 0.05, 6.0, 20, 20); // Adjusted length from 8.0 to 6.0 to fit between poles
	gluDeleteQuadric(quad);
	glPopMatrix();

	glPopMatrix();
}

// Initialize the timer function in your setup
void initializeAnimation() {
	glutTimerFunc(1000, togglePoleColors, 0); // Start the animation timer
}

void drawStartingBlocks() {
	// Set color for the base of the starting blocks
	glColor3f(0.3f, 0.3f, 0.3f); // Dark grey for the base

	// Draw the base plate
	glPushMatrix();
	glTranslated(0.0, 0.02, 1.0); // Position the base plate
	glScaled(1.5, 0.02, 0.5); // Scale to create a rectangular base plate
	glutSolidCube(1.0);
	glPopMatrix();

	// Set color for the foot pads of the starting blocks
	glColor3f(0.6f, 0.2f, 0.2f); // Dark red for the foot pads

	// Draw left foot pad
	glPushMatrix();
	glTranslated(-0.4, 0.1, 1.2); // Position the left foot pad
	glRotated(45, 1.0, 0.0, 0.0); // Rotate to angle the pad
	glScaled(0.3, 0.1, 0.05); // Scale to make it a flat angled pad
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw right foot pad
	glPushMatrix();
	glTranslated(0.4, 0.1, 1.2); // Position the right foot pad
	glRotated(45, 1.0, 0.0, 0.0); // Rotate to angle the pad
	glScaled(0.3, 0.1, 0.05); // Scale to make it a flat angled pad
	glutSolidCube(1.0);
	glPopMatrix();

	// Set color for adjustment bolts
	glColor3f(0.8f, 0.8f, 0.8f); // Light grey for adjustment bolts

	// Draw left adjustment bolt
	glPushMatrix();
	glTranslated(-0.4, 0.05, 1.4); // Position the left bolt near the foot pad
	glScaled(0.05, 0.05, 0.05); // Scale to make it a small bolt
	glutSolidSphere(0.5, 20, 20);
	glPopMatrix();

	// Draw right adjustment bolt
	glPushMatrix();
	glTranslated(0.4, 0.05, 1.4); // Position the right bolt near the foot pad
	glScaled(0.05, 0.05, 0.05); // Scale to make it a small bolt
	glutSolidSphere(0.5, 20, 20);
	glPopMatrix();
}

void drawDumbbells() {
	// Draw first dumbbell
	glColor3f(
		dumbellsColors1[dumbellsColorIndex1][0],
		dumbellsColors1[dumbellsColorIndex1][1],
		dumbellsColors1[dumbellsColorIndex1][2]
	);

	glPushMatrix();
	glTranslated(3.5, 0.1, 4.0); // Position the dumbbell on the ground to the right of the podium
	glRotatef(90.0, 0.0, 1.0, 0.0); // Rotate the whole dumbbell 90 degrees around the y-axis
	glTranslated(0.0, 0.05, 0.0); // Slight translation along the y-axis to move it up

	// Draw left weight
	glPushMatrix();
	glTranslated(-0.3, 0.0, 0.0); // Move the weight to the left of the bar
	glutSolidSphere(0.15, 20, 20); // Larger circle for the left weight
	glPopMatrix();

	// Draw right weight
	glPushMatrix();
	glTranslated(0.3, 0.0, 0.0); // Move the weight to the right of the bar
	glutSolidSphere(0.15, 20, 20); // Larger circle for the right weight
	glPopMatrix();

	// Draw bar (cylinder connecting weights)
	glPushMatrix();
	glTranslated(-0.3, 0.0, 0.0); // Shift cylinder left to align between weights
	glRotatef(90.0, 0.0, 1.0, 0.0); // Rotate the cylinder to align between the weights
	GLUquadric* bar = gluNewQuadric();
	gluCylinder(bar, 0.05, 0.05, 0.6, 20, 20); // Longer, thinner cylinder for the bar
	gluDeleteQuadric(bar);
	glPopMatrix();

	glPopMatrix();

	glColor3f(
		dumbellsColors2[dumbellsColorIndex2][0],
		dumbellsColors2[dumbellsColorIndex2][1],
		dumbellsColors2[dumbellsColorIndex2][2]
	);

	// Draw second dumbbell immediately beside the first one
	glPushMatrix();
	glTranslated(4.1, 0.1, 4.0); // Position the second dumbbell immediately beside the first one
	glRotatef(90.0, 0.0, 1.0, 0.0); // Rotate the whole dumbbell 90 degrees around the y-axis
	glTranslated(0.0, 0.05, 0.0); // Slight translation along the y-axis to move it up

	// Draw left weight for second dumbbell
	glPushMatrix();
	glTranslated(-0.3, 0.0, 0.0); // Move the weight to the left of the bar
	glutSolidSphere(0.15, 20, 20); // Larger circle for the left weight
	glPopMatrix();

	// Draw right weight for second dumbbell
	glPushMatrix();
	glTranslated(0.3, 0.0, 0.0); // Move the weight to the right of the bar
	glutSolidSphere(0.15, 20, 20); // Larger circle for the right weight
	glPopMatrix();

	// Draw bar (cylinder connecting weights) for second dumbbell
	glPushMatrix();
	glTranslated(-0.3, 0.0, 0.0); // Shift cylinder left to align between weights
	glRotatef(90.0, 0.0, 1.0, 0.0); // Rotate the cylinder to align between the weights
	GLUquadric* bar2 = gluNewQuadric();
	gluCylinder(bar2, 0.05, 0.05, 0.6, 20, 20); // Longer, thinner cylinder for the bar
	gluDeleteQuadric(bar2);
	glPopMatrix();

	glPopMatrix();
}

void drawMedalPodium() {
	// Draw the common base
	glColor3f(
		podiumColors[podiumColorIndex][0],
		podiumColors[podiumColorIndex][1],
		podiumColors[podiumColorIndex][2]
	);

	glPushMatrix();
	glTranslated(0.0, 0.1, 4.0); // Position the base on the ground
	glScaled(3.7, 0.2, 1.5);     // Scale the base to make it shorter
	glutSolidCube(1.0);
	glPopMatrix();

	// Set dynamic color for the podium blocks


	// Draw 1st place block (center, tallest)
	glPushMatrix();
	glTranslated(0.0, 0.3, 4.0); // Adjust height to sit on the base
	glScaled(1.0, 0.3, 1.0);     // Scale to make it the tallest
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw 2nd place block (left, slightly shorter)
	glPushMatrix();
	glTranslated(-1.2, 0.25, 4.0); // Adjust height to sit on the base
	glScaled(1.0, 0.2, 1.0);       // Scale to make it slightly shorter
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw 3rd place block (right, shortest)
	glPushMatrix();
	glTranslated(1.2, 0.2, 4.0); // Adjust height to sit on the base
	glScaled(1.0, 0.1, 1.0);     // Scale to make it the shortest
	glutSolidCube(1.0);
	glPopMatrix();
}

void drawTorchStand() {
	glPushMatrix();

	// Apply X-axis translation animation
	glTranslated(torchStandXOffset, 0.0, 0.0);

	// Set color for the stand base
	glColor3f(0.4f, 0.2f, 0.0f); // Brown for the base

	// Draw the base of the stand
	glPushMatrix();
	glTranslated(-4.0, 0.05, 4.5); // Move forward along the z-axis
	glScaled(0.5, 0.1, 0.5); // Scale to make a flat base
	glutSolidCube(1.0);
	glPopMatrix();

	// Set color for the stand pole
	glColor3f(1.0f, 0.0f, 1.0f); // Light grey for the pole
	// Magenta
		// Draw the stand pole
	glPushMatrix();
	glTranslated(-4.0, 0.55, 4.5); // Move forward along the z-axis
	glScaled(0.1, 1.0, 0.1); // Scale to make a tall, thin pole
	glutSolidCube(1.0);
	glPopMatrix();

	// Set color for the flame
	glColor3f(1.0f, 0.5f, 0.0f); // Orange for the flame

	// Draw the flame at the top of the stand
	glPushMatrix();
	glTranslated(-4.0, 1.1, 4.5); // Move forward along the z-axis
	glutSolidSphere(0.2, 20, 20); // Draw the flame as a sphere
	glPopMatrix();

	glPopMatrix();
}

void drawFieldWithLanes() {
	// Set the base color to purple for the track field
	glColor3f(1.0f, 0.0f, 0.0f);

	// Draw the base field (track)
	glPushMatrix();
	glTranslated(0.0, 0.01, 0.0); // Slightly above ground level to avoid z-fighting
	glScaled(12.0, 0.01, 6.0); // Scaled up from (8.0, 0.01, 4.0) to (12.0, 0.01, 6.0)
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw lane lines to create 5 lanes (4 lines)
	glColor3f(1.0f, 1.0f, 1.0f); // Set color to white for lane lines

	// Spacing between lanes, assuming the entire field is scaled to 6.0 in width (z-axis)
	float laneSpacing = 6.0f / 5.0f;

	for (int i = 1; i < 5; ++i) {
		glPushMatrix();
		glTranslated(0.0, 0.015, -3.0f + i * laneSpacing); // Translate along z-axis
		glScaled(12.0, 0.005, 0.05); // Length across the x-axis, increased width along z-axis
		glutSolidCube(1.0);
		glPopMatrix();
	}
}

void drawOlympicsLogoFlag() {
	glPushMatrix();
	// Apply translation to the entire flag
	glTranslatef(-3.0f, 0.0f, 0.0f);

	// Handle scaling animation
	if (animationActive) {
		// Translate to the origin for scaling
		glTranslatef(5.0f, 0.0f, -4.0f); // Move the flag base to the origin

		// Adjust translation to keep the bottom of the flagpole on the ground
		float adjustment = (1.0f - olympicFlagScale) * 0.5f; // Calculate upward adjustment for scaling
		glTranslatef(0.0f, -adjustment, 0.0f); // Compensate for scaling

		glScalef(olympicFlagScale, olympicFlagScale, olympicFlagScale); // Scale at the origin

		// Move it back to its original position
		glTranslatef(-5.0f, adjustment, 4.0f);
	}

	// Set color for the flagpole
	glColor3f(1.0f, 0.5f, 0.0f); // Light grey for the pole

	// Draw the flagpole
	glPushMatrix();
	glTranslated(5.0, 0.5, -4.0); // Position the pole
	glScaled(0.05, 1.0, 0.05);
	glutSolidCube(1.0);
	glPopMatrix();

	// Set color for the white flag
	glColor3f(0.0f, 1.0f, 0.0f); // White
	glPushMatrix();
	glTranslated(5.0, 1.0, -4.0);
	glScaled(1.2, 0.6, 0.05);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw Olympic rings
	float ringRadius = 0.1;
	float ringTubeRadius = 0.02;

	// Blue ring
	glColor3f(0.0f, 0.0f, 1.0f);
	glPushMatrix();
	glTranslated(4.7, 1.1, -3.95);
	glutSolidTorus(ringTubeRadius, ringRadius, 20, 20);
	glPopMatrix();

	// Black ring
	glColor3f(0.0f, 0.0f, 0.0f);
	glPushMatrix();
	glTranslated(5.0, 1.1, -3.95);
	glutSolidTorus(ringTubeRadius, ringRadius, 20, 20);
	glPopMatrix();

	// Red ring
	glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();
	glTranslated(5.3, 1.1, -3.95);
	glutSolidTorus(ringTubeRadius, ringRadius, 20, 20);
	glPopMatrix();

	// Yellow ring
	glColor3f(1.0f, 1.0f, 0.0f);
	glPushMatrix();
	glTranslated(4.85, 1.0, -3.95);
	glutSolidTorus(ringTubeRadius, ringRadius, 20, 20);
	glPopMatrix();

	// Green ring
	glColor3f(0.0f, 1.0f, 0.0f);
	glPushMatrix();
	glTranslated(5.15, 1.0, -3.95);
	glutSolidTorus(ringTubeRadius, ringRadius, 20, 20);
	glPopMatrix();

	glPopMatrix();
}

void drawRunner() {
	glPushMatrix();

	// Position the player on the floor
	glTranslatef(playerPosition.x, playerPosition.y, playerPosition.z);
	glRotatef(playerRotation, 0.0f, 1.0f, 0.0f);

	// Draw the head with face details (front with eyes)
	glPushMatrix();
	glTranslated(0.0, 0.55, 0.0); // Lowered head to align with the torso
	glColor3f(1.0f, 0.8f, 0.6f); // Skin color for the head
	glutSolidSphere(0.05, 15, 15); // Head sphere
	// Draw eyes
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.0f); // Black for the eyes
	glTranslated(-0.02, 0.01, 0.045); // Left eye position
	glutSolidSphere(0.005, 10, 10);
	glTranslated(0.04, 0.0, 0.0); // Right eye position
	glutSolidSphere(0.005, 10, 10);
	glPopMatrix();
	glPopMatrix();

	// Draw the torso (front)
	glPushMatrix();
	glTranslated(0.0, 0.4, 0.0); // Lowered torso to align with the legs
	glScaled(0.1, 0.2, 0.05); // Scale for the torso
	glColor3f(0.8f, 0.0f, 0.0f); // Red color for the front
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the back of the torso
	glPushMatrix();
	glTranslated(0.0, 0.4, -0.025); // Lowered back of the torso
	glScaled(0.1, 0.2, 0.01); // Thinner for the back
	glColor3f(0.2f, 0.2f, 0.2f); // Dark grey color for the back
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the left leg
	glPushMatrix();
	glTranslated(-0.025, 0.15, 0.0); // Left leg position
	glScaled(0.025, 0.3, 0.025); // Scale for the leg
	glColor3f(0.4f, 0.4f, 0.4f); // Grey for the legs
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the right leg
	glPushMatrix();
	glTranslated(0.025, 0.15, 0.0); // Right leg position
	glScaled(0.025, 0.3, 0.025); // Scale for the leg
	glColor3f(0.4f, 0.4f, 0.4f); // Grey for the legs
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the left arm (lowered and attached to torso)
	glPushMatrix();
	glTranslated(-0.08, 0.4, -0.02); // Lowered the arm to align with the torso
	glRotatef(-20.0, 0.0, 0.0, 1.0); // Slight rotation to attach diagonally
	glScaled(0.025, 0.15, 0.025); // Scale for the arm
	glColor3f(0.8f, 0.0f, 0.0f); // Red color for the arm
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the right arm (lowered and attached to torso)
	glPushMatrix();
	glTranslated(0.08, 0.4, -0.02); // Lowered the arm to align with the torso
	glRotatef(20.0, 0.0, 0.0, 1.0); // Slight rotation to attach diagonally
	glScaled(0.025, 0.15, 0.025); // Scale for the arm
	glColor3f(0.8f, 0.0f, 0.0f); // Red color for the arm
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the left shoe
	glPushMatrix();
	glTranslated(-0.025, 0.0, 0.0); // Left shoe touches the ground
	glScaled(0.025, 0.05, 0.05); // Scale for the shoe
	glColor3f(0.0f, 0.0f, 0.0f); // Black for the shoes
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw the right shoe
	glPushMatrix();
	glTranslated(0.025, 0.0, 0.0); // Right shoe touches the ground
	glScaled(0.025, 0.05, 0.05); // Scale for the shoe
	glColor3f(0.0f, 0.0f, 0.0f); // Black for the shoes
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void renderTimer() {
	// Save the current projection and model-view matrices
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// Set up orthographic projection (screen space coordinates)
	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
	gluOrtho2D(0, windowWidth, 0, windowHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Set the color for the timer text
	glColor3f(0.0f, 0.0f, 0.0f); // Black color for text

	// Position the text at the top-right corner
	glRasterPos2f(windowWidth - 150, windowHeight - 30); // Adjust offsets for padding

	// Construct the timer string
	char timerText[50];
	snprintf(timerText, sizeof(timerText), "Timer: %d seconds", gameTimeRemaining);

	// Render the text
	for (const char* c = timerText; *c != '\0'; ++c) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}

	// Restore the original projection and model-view matrices
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

// Function to reset to the normal view (initial camera setup)
void resetToNormalView() {
	camera.eye = Vector3f(1.0f, 1.0f, 1.0f);       // Reset eye position
	camera.center = Vector3f(0.0f, 0.0f, 0.0f);    // Reset target position
	camera.up = Vector3f(0.0f, 1.0f, 0.0f);        // Reset up vector
	setupCamera();                                 // Apply the new setup
}

void Display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (gameOver) {
		// Save the current projection and model-view matrices
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, 1, 0, 1); // Set up orthographic projection (screen space coordinates)

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity(); // Reset model-view matrix

		// Center the text in the screen
		if (gameWon) {
			gameWonSound.stop();
			// Display "Game Won" screen
			glColor3f(0.0, 1.0, 0.0); // Green text
			glRasterPos2f(0.4, 0.5); // Center text at (0.5, 0.5) in screen space
			const char* message = "Game Won!";
			for (const char* c = message; *c != '\0'; ++c) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
			}
		}
		else {

			gameWonSound.stop();

			// Display "Game Lost" screen
			glColor3f(1.0, 0.0, 0.0); // Red text
			glRasterPos2f(0.4, 0.5); // Center text at (0.5, 0.5) in screen space
			const char* message = "Game Lost!";
			for (const char* c = message; *c != '\0'; ++c) {
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
			}
		}

		// Restore the original projection and model-view matrices
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glFlush();
		return; // Stop further rendering
	}


	setupCamera();
	setupLights();


	glPushMatrix();
	drawFieldWithLanes();  // Draw the larger field with lanes
	glPopMatrix();


	glPushMatrix();
	drawFloor();  // Draw the larger field with lanes
	glPopMatrix();

	glPushMatrix();
	drawDumbbells();  // Draw the larger field with lanes
	glPopMatrix();

	glPushMatrix();
	drawWalls();
	glPopMatrix();


	glPushMatrix();
	drawFinishLine();  // Draw the rotated finish line
	glPopMatrix();

	glPushMatrix();
	drawEgyptianFlag();  // Draw the red flag
	glPopMatrix();


	glPushMatrix();
	drawOlympicsLogoFlag();  // Draw the Olympics logo flag
	glPopMatrix();

	glPushMatrix();
	drawMedalPodium();  // Draw the medal podium
	glPopMatrix();

	glPushMatrix();
	drawTorchStand();  // Draw the Olympic torch stand
	glPopMatrix();

	glPushMatrix();
	drawRunner();
	glPopMatrix();

	// Draw the timer text

	glPushMatrix();
	renderTimer();
	glPopMatrix();

	glFlush();
}

void Keyboard(unsigned char key, int x, int y) {
	float d = 0.07;
	switch (key) {
	case 'w':
		camera.moveY(d);
		break;
	case 's':
		camera.moveY(-d);
		break;
	case 'a':
		camera.moveX(d);
		break;
	case 'd':
		camera.moveX(-d);
		break;
	case 'q':
		camera.moveZ(d);
		break;
	case 'e':
		camera.moveZ(-d);
		break;
	case '1': // Top view
		setView("top");
		currentView = "top";
		break;
	case '2': // Side view
		setView("side");
		currentView = "side";
		break;
	case '3': // Front view
		setView("front");
		currentView = "front";
		break;
	case '4': // Reset to normal view
		resetToNormalView();
		currentView = "normal";
		break;
	case 'h': // Toggle animation
		animationActive = !animationActive;
		if (!animationActive) {
			torchStandXOffset = 0.0f; // Reset to original position
			glutPostRedisplay(); // Ensure the scene is updated
			gameWonSound.stop();
		}
		else {


			if (!gameWonSoundBuffer.loadFromFile("C:\\Users\\samir\\Downloads\\3D Game\\Debug\\gameWon.wav")) {
				std::cerr << "Error loading game won sound" << std::endl;
			}
			else {
				std::cout << "Game won sound loaded" << std::endl;
				gameWonSound.setBuffer(gameWonSoundBuffer);
				gameWonSound.setLoop(true); // Enable looping for the sound
				gameWonSound.play();
			}

			glutTimerFunc(16, updateAnimations, 0); // Restart the animation timer
		}
		break;
	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	}

	glutPostRedisplay();
}

void handleCollision() {
	// Example logic: Print a message or implement specific behavior
	printf("Collision with wall detected!\n");

	if (!collideSoundBuffer.loadFromFile("C:\\Users\\samir\\Downloads\\3D Game\\Debug\\collide.wav")) {
		std::cerr << "Error loading collide sound" << std::endl;
	}
	else {
		std::cout << "Collide soun loaded" << std::endl;
		collectibleSound.setBuffer(collideSoundBuffer);
		collectibleSound.play();
	}

	// You can add other actions here, such as reducing player health, playing a sound, etc.
}

void updateTimer(int value) {
	if (gameRunning) {
		if (gameTimeRemaining > 0) {
			gameTimeRemaining--;
			glutTimerFunc(1000, updateTimer, 0);  // Set timer for the next second
		}
		else {
			// Timer expired without reaching the finish line
			gameRunning = false;
			gameOver = true;
		}
		glutPostRedisplay();
	}
}

void Special(int key, int x, int y) {
	if (gameOver) return; // Stop processing if the game is over

	float d = 0.05; // Movement distance
	float wallLeft = -8.0 + 0.05;  // Updated left wall boundary
	float wallRight = 8.0 - 0.05;  // Updated right wall boundary
	float wallBack = -5.0 + 0.05;  // Back wall boundary remains the same
	float wallFront = 5.0 - 0.05;  // Front wall boundary remains the same

	// Store the initial position for potential collision rollback
	float prevX = playerPosition.x;
	float prevZ = playerPosition.z;

	// Move the player based on the key press
	switch (key) {
	case GLUT_KEY_UP:
		if (currentView == "side") {
			playerPosition.x -= d; // Move left
			playerRotation = -90.0f;
		}
		else {
			playerPosition.z -= d; // Move forward (towards the back wall)
			playerRotation = 180.0f;
		}

		break;
	case GLUT_KEY_DOWN:
		if (currentView == "side") {
			playerPosition.x += d; // Move right
			playerRotation = 90.0f;
		}
		else {
			playerPosition.z += d; // Move backward (towards the front wall)
			playerRotation = 0.0f;
		}

		break;
	case GLUT_KEY_LEFT:
		if (currentView == "side") {
			playerPosition.z += d; // Move backward (towards the front wall)
			playerRotation = 0.0f;
		}
		else {
			playerPosition.x -= d; // Move left
			playerRotation = -90.0f;
		}

		break;
	case GLUT_KEY_RIGHT:
		if (currentView == "side") {
			playerPosition.z -= d; // Move forward (towards the back wall)
			playerRotation = 180.0f;
		}
		else {

			playerPosition.x += d; // Move right
			playerRotation = 90.0f;
		}
		break;
	}

	// Check for collisions with the walls
	if (playerPosition.x < wallLeft || playerPosition.x > wallRight ||
		playerPosition.z < wallBack || playerPosition.z > wallFront) {
		// Collision detected: reset position to previous position
		playerPosition.x = prevX;
		playerPosition.z = prevZ;

		// Implement additional logic for collision
		handleCollision();
	}

	// Check for collision with the finish line's cylinder
	if (finishLineVisible && playerPosition.x <= -5.0f && playerPosition.z <= 3.0f && playerPosition.z >= -3.0f) { // Z range of the cylinder
		finishLineVisible = false; // Hide the entire finish line

		gameWon = true;
		glutPostRedisplay();
	}

	glutPostRedisplay();
}

void main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutTimerFunc(2000, toggleWallColor, 0);
	glutInitWindowSize(1100, 600);
	glutInitWindowPosition(120, 80);

	// Initialize the timer
	glutTimerFunc(1000, updateTimer, 0);


	glutCreateWindow("Olympic Runner");
	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(Special);

	if (!backgroundMusicBuffer.loadFromFile("C:\\Users\\samir\\Downloads\\3D Game\\Debug\\backgroundMusic.wav")) {
		std::cerr << "Error loading background music" << std::endl;
	}
	else {
		std::cout << "Background music loaded" << std::endl;
		backgroundMusic.setBuffer(backgroundMusicBuffer);
		backgroundMusic.setLoop(true);  // Loop the background music
		backgroundMusic.play();
	}

	initializeAnimation();

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}
