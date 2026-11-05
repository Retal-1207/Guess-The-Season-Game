//////////
//Libraries//
 //////////
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>      // handle the window-managing operations
#else
#include <GL/glew.h>
#include <GL/freeglut.h>    // handle the window-managing operations
#endif
#include <stdlib.h>
#include <iostream>
#include <string>

// NEW: Windows sound library for background music.
// Source: Microsoft PlaySound documentation: https://learn.microsoft.com/en-us/windows/win32/api/mmsystem/nf-mmsystem-playsound
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

using namespace std;


//////////
//Variables//
 //////////
int windowWidth = 1024; int windowHeight = 600; int gameState = 0; GLdouble PI = 3.14159265359; int triangleAmount = 200; float twicePi = 2.0f * PI;
// NEW: Animation timer for butterfly movement.
float butterflyTime = 0.0f;
// NEW: Zoom value for 2D viewing/zooming using keyboard (+ / -).
float zoom = 1.0f; float sunAngle = 0.0f; const char* feedbackMessage = "";
float charX = 0.0f; // Horizontal movement
float charY = 0.0f; // Vertical movement
float trophyRotation = 0.0f; // Variable to control trophy rotation
float transValueX = 3.5f;
float transValueY = 2.0f;
float transValueZ = 2.0f;
float nearValue = 0.01f;
float farValue = 50.0f;
float windmillAngle = 0.0f; // Variable to control windmill blades rotation

/////////////
//Screen cases//
 /////////////
#define Start_Screen 0
#define Winter 1
#define Spring 2
#define Summer 3
#define Autumn 4
#define End_Screen 5


 /////////
//Texture//
 ////////

GLuint winterTexture, spring1Texture, spring2Texture, spring3Texture, autumnTexture;
char winterPath[] = "assets\\textures\\winter.bmp"; char spring1Path[] = "assets\\textures\\FloralTexture.bmp"; char spring2Path[] = "assets\\textures\\PinkTexture.bmp";
char spring3Path[] = "assets\\textures\\RiverTexture.bmp"; char autumnPath[] = "assets\\textures\\autumn.bmp";

// Function to load BMP images and convert them to OpenGL textures
GLuint LoadTexture(const char* filename, int width, int height)
{
    GLuint texture;
    unsigned char* data;
    FILE* file;

    //The following code will read in our RAW file
    //file = fopen(filename, "rb");
    fopen_s(&file, filename, "rb");

    if (file == NULL)
    {
        cout << "Warning: Texture not found -> " << filename << endl;
        return 0;
    }

    data = (unsigned char*)malloc(width * height * 3);
    fseek(file, 54, SEEK_SET); // Skip BMP header (54 bytes)
    fread(data, width * height * 3, 1, file);
    fclose(file);

    // reorder the image colors to RGB not BGR
    for (int i = 0; i < width * height; ++i)
    {
        int index = i * 3;
        unsigned char B, R;
        B = data[index];
        R = data[index + 2];

        data[index] = R;
        data[index + 2] = B;

    }

    glGenTextures(1, &texture);            //generate the texture with the loaded data
    glBindTexture(GL_TEXTURE_2D, texture); //bind the texture to it's array

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);  //set texture environment parameters

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    free(data); //free the texture array

    if (glGetError() != GL_NO_ERROR)
        printf("GLError in genTexture()\n");

    return texture; //return whether it was successfull 
}

///////////////
//Basic functions//
 ////////////////


// NEW: Helper functions to reduce type-conversion warnings in OpenGL calls.
// Source: OpenGL fixed-function transformation reference: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/
// ----------------------------- Safe OpenGL Helpers -----------------------------
void V2(double x, double y) { glVertex2f(static_cast<GLfloat>(x), static_cast<GLfloat>(y)); }
void C3(double r, double g, double b) { glColor3f(static_cast<GLfloat>(r), static_cast<GLfloat>(g), static_cast<GLfloat>(b)); }
void C4(double r, double g, double b, double a) { glColor4f(static_cast<GLfloat>(r), static_cast<GLfloat>(g), static_cast<GLfloat>(b), static_cast<GLfloat>(a)); }
void T2(double u, double v) { glTexCoord2f(static_cast<GLfloat>(u), static_cast<GLfloat>(v)); }
void Translate(double x, double y, double z) { glTranslatef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z)); }
void Scale(double x, double y, double z) { glScalef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z)); }
void Rotate(double angle, double x, double y, double z) { glRotatef(static_cast<GLfloat>(angle), static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z)); }

// Function to write text
void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// Circle
void drawCircle(GLfloat x, GLfloat y, GLfloat radius) {
    int i;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for (i = 0; i <= triangleAmount; i++) {
        glVertex2f(
            x + (radius * cos(i * twicePi / triangleAmount)),
            y + (radius * sin(i * twicePi / triangleAmount))
        );
    }

    glEnd();
}

// NEW: Ellipse helper used for clouds, flowers, and butterfly wings.
void drawEllipse(double x, double y, double rx, double ry) {
    glBegin(GL_TRIANGLE_FAN);
    V2(x, y);
    for (int i = 0; i <= triangleAmount; i++) {
        double angle = static_cast<double>(i) * twicePi / static_cast<double>(triangleAmount);
        V2(x + rx * cos(angle), y + ry * sin(angle));
    }
    glEnd();
}



//////////
//character//
 //////////
//Face
void DrawFace()
{
    glColor3f(1.0, 0.9, 0.8);
    drawCircle(0.0, 0.0, 0.5);
}

//Back hair  
void DrawBackHair()
{
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.75f, -0.25f);

    glVertex2f(-0.73f, 0.05f); glVertex2f(-0.70f, 0.25f); glVertex2f(-0.65f, 0.35f); glVertex2f(-0.60f, 0.45f); glVertex2f(-0.50f, 0.55f); glVertex2f(-0.45f, 0.60f); glVertex2f(-0.35f, 0.65f); glVertex2f(-0.25f, 0.70f);
    glVertex2f(0.0f, 0.75f);     // Middle of the head 
    glVertex2f(0.25f, 0.70f); glVertex2f(0.35f, 0.65f); glVertex2f(0.45f, 0.60f); glVertex2f(0.50f, 0.55f); glVertex2f(0.60f, 0.45f); glVertex2f(0.65f, 0.35f); glVertex2f(0.70f, 0.25f); glVertex2f(0.73f, 0.05f);

    glVertex2f(0.75f, -0.25f);
    glEnd();
}

// Top hair
void DrawTopHairSimple()
{
    glColor3f(0.1f, 0.1f, 0.1f);
    // Middle circle  
    drawCircle(0.0f, 0.47f, 0.22f); // x, y, radius
    // Left circle 
    drawCircle(-0.28f, 0.47f, 0.20f);
    // Right circle
    drawCircle(0.28f, 0.47f, 0.20f);
}

// Bangs
void DrawBangs()
{
    glColor3f(0.1f, 0.1f, 0.1f);
    // Triangles for bangs
    glBegin(GL_TRIANGLES);
    // left
    glVertex2f(-0.50f, 0.32f); glVertex2f(-0.30f, 0.32f); glVertex2f(-0.40f, 0.12f);
    // mid left
    glVertex2f(-0.30f, 0.32f); glVertex2f(-0.08f, 0.32f); glVertex2f(-0.19f, 0.08f);
    // middle
    glVertex2f(-0.12f, 0.34f); glVertex2f(0.12f, 0.34f); glVertex2f(0.0f, 0.05f);
    // mid right
    glVertex2f(0.08f, 0.32f); glVertex2f(0.30f, 0.32f); glVertex2f(0.19f, 0.08f);
    // right
    glVertex2f(0.30f, 0.32f); glVertex2f(0.50f, 0.32f); glVertex2f(0.40f, 0.12f);

    glEnd();
}

//Ear
void DrawEarLeft()
{
    glColor3f(1.0f, 0.9f, 0.8f);
    drawCircle(-0.52f, -0.07f, 0.09f);
}

//Ear
void DrawEarRight()
{
    glColor3f(1.0f, 0.9f, 0.8f);
    drawCircle(0.52f, -0.07f, 0.09f);
}

//Eye
void DrawEyes()
{
    glColor3f(0.0, 0.0, 0.0);
    drawCircle(-0.15f, -0.04f, 0.07f);// Left eye
    drawCircle(0.15f, -0.04f, 0.07f);// Right eye
    // Eye shine
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(-0.13f, -0.01f, 0.025f);// left
    drawCircle(0.17f, -0.01f, 0.025f);// right
}

// Eyebrow
void DrawEyebrows()
{
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(0.1f);
    // Left eyebrow
    glBegin(GL_LINE_STRIP);
    glVertex2f(-0.30f, 0.05f); glVertex2f(-0.27f, 0.07f); glVertex2f(-0.24f, 0.08f); glVertex2f(-0.20f, 0.09f); glVertex2f(-0.16f, 0.08f); glVertex2f(-0.12f, 0.07f); glVertex2f(-0.10f, 0.05f);
    glEnd();
    // Right eyebrow
    glBegin(GL_LINE_STRIP);
    glVertex2f(0.10f, 0.05f); glVertex2f(0.12f, 0.07f); glVertex2f(0.16f, 0.08f); glVertex2f(0.20f, 0.09f); glVertex2f(0.24f, 0.08f); glVertex2f(0.27f, 0.07f); glVertex2f(0.30f, 0.05f);
    glEnd();
}

// Cheeks
void DrawCheeks()
{
    glColor3f(1.0f, 0.6f, 0.6f);
    // Left cheek
    drawCircle(-0.30f, -0.31f, 0.07f);
    // Right cheek
    drawCircle(0.30f, -0.31f, 0.07f);
}

// Smile
void DrawSmile()
{
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(0.1f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(-0.08f, -0.28f); glVertex2f(-0.05f, -0.30f); glVertex2f(0.0f, -0.31f); glVertex2f(0.05f, -0.30f); glVertex2f(0.08f, -0.28f);
    glEnd();
}

// Neck
void drawNeck()
{
    glColor3f(1.0, 0.9, 0.8);
    glBegin(GL_POLYGON);
    glVertex2f(-0.05f, -0.50f); glVertex2f(0.05f, -0.50f); glVertex2f(0.05f, -0.60f); glVertex2f(-0.05f, -0.60f);
    glEnd();
}

// Torso (Shirt)
void drawTorso() {
    // This function is for start/end screens and winter only
    if (gameState != Start_Screen && gameState != End_Screen && gameState != Winter)
        return;
    // For Winter
    if (gameState == Winter) {
        glColor3f(1.0f, 1.0f, 0.6f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f); // Start and End screens
    }
    glBegin(GL_POLYGON);
    glVertex2f(-0.35f, -0.60f); glVertex2f(0.35f, -0.60f); glVertex2f(0.30f, -1.25f); glVertex2f(-0.30f, -1.25f);
    glEnd();

    // Suspender straps
    glColor3f(1.0f, 0.0f, 0.0f);
    // left
    glBegin(GL_POLYGON);
    glVertex2f(-0.20f, -0.60f); glVertex2f(-0.12f, -0.60f); glVertex2f(-0.12f, -1.25f); glVertex2f(-0.20f, -1.25f);
    glEnd();
    // right
    glBegin(GL_POLYGON);
    glVertex2f(0.12f, -0.60f); glVertex2f(0.20f, -0.60f); glVertex2f(0.20f, -1.25f); glVertex2f(0.12f, -1.25f);
    glEnd();
}

// Torso (Autumn shirt)
void drawAutumnTorso()
{
    if (gameState == Autumn) {
        // Base brown shirt
        glColor3f(0.72f, 0.45f, 0.20f);
        glBegin(GL_POLYGON);
        glVertex2f(-0.38f, -0.60f); glVertex2f(0.38f, -0.60f); glVertex2f(0.32f, -1.28f); glVertex2f(-0.32f, -1.28f);
        glEnd();
        // Collar 
        glColor3f(0.90f, 0.55f, 0.08f);
        glBegin(GL_POLYGON);
        glVertex2f(-0.18f, -0.60f); glVertex2f(0.18f, -0.60f); glVertex2f(0.08f, -0.78f); glVertex2f(-0.08f, -0.78f);
        glEnd();
        // Left pocket
        glColor3f(0.80f, 0.30f, 0.02f);
        glBegin(GL_POLYGON);
        glVertex2f(-0.14f, -0.78f); glVertex2f(-0.04f, -0.78f); glVertex2f(-0.05f, -1.32f); glVertex2f(-0.15f, -1.32f);
        glEnd();
        // Right pocket
        glBegin(GL_POLYGON);
        glVertex2f(0.04f, -0.78f); glVertex2f(0.14f, -0.78f); glVertex2f(0.15f, -1.32f); glVertex2f(0.05f, -1.32f);
        glEnd();
        // Line details on the shirt
        glColor3f(0.55f, 0.30f, 0.12f);
        glLineWidth(1.3f);
        glBegin(GL_LINES);
        glVertex2f(-0.20f, -0.65f); glVertex2f(-0.22f, -1.25f); glVertex2f(0.0f, -0.62f);   glVertex2f(0.0f, -1.25f); glVertex2f(0.20f, -0.65f);  glVertex2f(0.22f, -1.25f);
        glEnd();
    }
}

// Torso (Summer shirt)
void drawSummerTorso()
{
    if (gameState == Summer) {
        //Pink Top
        glColor3f(1.0f, 0.7f, 0.8f);
        glBegin(GL_POLYGON);
        glVertex2f(-0.35f, -0.60f); glVertex2f(0.35f, -0.60f); glVertex2f(0.30f, -1.25f); glVertex2f(-0.30f, -1.25f);
        glEnd();
        //Yellow Dots
        glColor3f(1.0f, 1.0f, 0.0f);
        drawCircle(-0.15f, -0.80f, 0.04f); drawCircle(0.15f, -0.95f, 0.04f); drawCircle(-0.05f, -1.10f, 0.04f); drawCircle(0.10f, -0.75f, 0.04f);
    }
}

// Scarf
void drawScarf()
{
    if (gameState == Winter) {
        glColor3f(0.9f, 0.0f, 0.0f);
        // Scarf wrapped around neck
        glBegin(GL_POLYGON);
        glVertex2f(-0.15f, -0.50f); glVertex2f(0.15f, -0.50f); glVertex2f(0.20f, -0.65f); glVertex2f(-0.20f, -0.65f);
        glEnd();
        // Hanging part
        glBegin(GL_POLYGON);
        glVertex2f(0.10f, -0.65f); glVertex2f(0.18f, -0.65f); glVertex2f(0.18f, -0.90f); glVertex2f(0.10f, -0.90f);
        glEnd();
    }
}

// Legs
void drawLegs() {
    // Like long socks
    if (gameState == Winter) {
        glColor3f(0.5, 0.5, 0.5);
    }
    else {
        glColor3f(1.0, 0.9, 0.8);
    }
    // left
    glBegin(GL_POLYGON);
    glVertex2f(-0.20f, -1.75f); glVertex2f(-0.05f, -1.75f); glVertex2f(-0.05f, -2.45f); glVertex2f(-0.20f, -2.45f);
    glEnd();
    // right
    glBegin(GL_POLYGON);
    glVertex2f(0.05f, -1.75f); glVertex2f(0.20f, -1.75f); glVertex2f(0.20f, -2.45f); glVertex2f(0.05f, -2.45f);
    glEnd();
}

// Skirt
void drawSkirt() {
    if (gameState == Autumn) {
        // Autumn = textured skirt
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, autumnTexture);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_POLYGON);
        glTexCoord2f(0.0, 0.0); glVertex2f(-0.35f, -1.25f); glTexCoord2f(1.0, 0.0); glVertex2f(0.35f, -1.25f); glTexCoord2f(1.0, 1.0); glVertex2f(0.55f, -1.95f); glTexCoord2f(0.0, 1.0); glVertex2f(-0.55f, -1.95f);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    else {
        glColor3f(0.9f, 0.0f, 0.0f);
        glBegin(GL_POLYGON);
        glVertex2f(-0.35f, -1.25f); glVertex2f(0.35f, -1.25f); glVertex2f(0.55f, -1.95f); glVertex2f(-0.55f, -1.95f);
        glEnd();
        // Draw pleats on the skirt  
        glColor3f(0.05f, 0.05f, 0.05f);
        glLineWidth(1.2);
        glBegin(GL_LINES);
        glVertex2f(-0.15f, -1.25f); glVertex2f(-0.25f, -1.95f); glVertex2f(0.0f, -1.25f); glVertex2f(0.0f, -1.95f); glVertex2f(0.15f, -1.25f);  glVertex2f(0.25f, -1.95f);
        glEnd();
    }
}

// Summer skirt
void drawSummerSkirt()
{
    if (gameState == Summer) {
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_POLYGON);
        glVertex2f(-0.35f, -1.25f); glVertex2f(0.35f, -1.25f); glVertex2f(0.55f, -1.95f); glVertex2f(-0.55f, -1.95f);
        glEnd();
    }
}

// Socks and shoes
void drawFeet() {
    glColor3f(1.0f, 1.0f, 1.0f);
    // Left sock (small rectangle above shoe)
    glBegin(GL_POLYGON);
    glVertex2f(-0.20f, -2.35f); glVertex2f(-0.05f, -2.35f); glVertex2f(-0.05f, -2.50f); glVertex2f(-0.20f, -2.50f);
    glEnd();
    // Right sock
    glBegin(GL_POLYGON);
    glVertex2f(0.05f, -2.35f); glVertex2f(0.20f, -2.35f); glVertex2f(0.20f, -2.50f); glVertex2f(0.05f, -2.50f);
    glEnd();

    // Left shoe
    if (gameState == Autumn) {
        glColor3f(0.72f, 0.43f, 0.16f);
    }
    else if (gameState == Summer) {
        glColor3f(1.0f, 0.7f, 0.8f);
    }
    // Spring case
    else if (gameState == Spring) {
        glColor3f(1.0f, 0.45f, 0.65f);
    }
    else {
        glColor3f(0.8f, 0.0f, 0.0f);
    }
    glBegin(GL_POLYGON);
    glVertex2f(-0.28f, -2.50f); glVertex2f(-0.07f, -2.50f); glVertex2f(-0.05f, -2.70f); glVertex2f(-0.30f, -2.70f);
    glEnd();

    // Right shoe
    if (gameState == Autumn) {
        glColor3f(0.72f, 0.43f, 0.16f);
    }
    else if (gameState == Summer) {
        glColor3f(1.0f, 0.7f, 0.8f);
    }
    // Spring case
    else if (gameState == Spring) {
        glColor3f(1.0f, 0.45f, 0.65f);
    }
    else {
        glColor3f(0.8f, 0.0f, 0.0f);
    }
    glBegin(GL_POLYGON);
    glVertex2f(0.07f, -2.50f); glVertex2f(0.28f, -2.50f); glVertex2f(0.30f, -2.70f); glVertex2f(0.05f, -2.70f);
    glEnd();
}

// Sleeves and hands
void drawArms() {
    glColor3f(1.0f, 1.0f, 1.0f);
    // Left sleeve (covers shoulder area)
     // Winter case
    if (gameState == Winter) {
        glColor3f(1.0f, 1.0f, 0.6f);
    }
    // Other seasons
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    glBegin(GL_POLYGON);
    glVertex2f(-0.35f, -0.60f); glVertex2f(-0.52f, -0.70f); glVertex2f(-0.52f, -1.05f); glVertex2f(-0.30f, -1.05f);
    glEnd();
    // Right sleeve
    // Winter case
    if (gameState == Winter) {
        glColor3f(1.0f, 1.0f, 0.6f);
    }
    // Other seasons
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    glBegin(GL_POLYGON);
    glVertex2f(0.35f, -0.60f); glVertex2f(0.52f, -0.70f); glVertex2f(0.52f, -1.05f); glVertex2f(0.30f, -1.05f);
    glEnd();

    // Hands
    glColor3f(1.0, 0.9, 0.8);
    // Left hand
    glBegin(GL_POLYGON);
    glVertex2f(-0.35f, -1.05f); glVertex2f(-0.48f, -1.05f); glVertex2f(-0.48f, -1.50f); glVertex2f(-0.35f, -1.50f);
    glEnd();
    // Right hand
    glBegin(GL_POLYGON);
    glVertex2f(0.35f, -1.05f); glVertex2f(0.48f, -1.05f); glVertex2f(0.48f, -1.50f); glVertex2f(0.35f, -1.50f);
    glEnd();
}

// Summer sunglasses
void drawSunglasses() {
    if (gameState == Summer) {
        glColor3f(0.0f, 0.0f, 0.0f);
        drawCircle(-0.16f, -0.04f, 0.11f); // Left Lens
        drawCircle(0.16f, -0.04f, 0.11f);  // Right Lens
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        glVertex2f(-0.05f, -0.04f); glVertex2f(0.05f, -0.04f); // Bridge
        glVertex2f(-0.27f, -0.04f); glVertex2f(-0.52f, -0.08f); // Left temple
        glVertex2f(0.27f, -0.04f); glVertex2f(0.52f, -0.08f);  // Right temple
        glEnd();
    }
}

// Ice Cream 
void drawIceCream() {
    // Summer case
    if (gameState == Summer) {
        // Cone
        glColor3f(0.8f, 0.5f, 0.2f);
        glBegin(GL_TRIANGLES);
        glVertex2f(0.38f, -0.90f); glVertex2f(0.50f, -0.90f); glVertex2f(0.44f, -1.35f);
        glEnd();
        // Scoop
        glColor3f(1.0f, 0.5f, 0.7f);
        drawCircle(0.44f, -0.85f, 0.13f);
    }
}

// Function to assemble all character parts
void MyMainObj()
{
    glPushMatrix();
    glTranslatef(300.0f + charX, 350.0f + charY, 0.0f); // Character position
    glScalef(80, 80, 1);       // Character size

    DrawBackHair(); DrawEarLeft(); DrawEarRight(); DrawFace(); DrawTopHairSimple(); DrawBangs(); DrawEyes(); DrawEyebrows(); DrawCheeks(); DrawSmile(); drawSunglasses();
    drawNeck(); drawScarf(); drawTorso(); drawAutumnTorso(); drawSummerTorso(); drawArms(); drawLegs(); drawSkirt(); drawSummerSkirt(); drawFeet(); drawIceCream();
    glPopMatrix();
}



////////////
//environment//
 ////////////

// Sun
void drawAttractiveSun(float cx, float cy, float r) {
    glPushMatrix();
    glTranslatef(cx, cy, 0);       // 1. Move to sun center
    glRotatef(sunAngle, 0, 0, 1);   // 2. Rotate
    glTranslatef(-cx, -cy, 0);     // 3. Move back
    // Back glow
    glBegin(GL_TRIANGLE_FAN);
    // Autumn case
    if (gameState == Autumn) {
        glColor4f(0.95f, 0.70f, 0.20f, 0.18f);
    }
    // Other seasons
    else {
        glColor4f(1.0f, 0.9f, 0.0f, 0.3f);
    }

    glVertex2f(cx, cy);
    // Autumn case
    if (gameState == Autumn) {
        glColor4f(0.95f, 0.70f, 0.20f, 0.0f);
    }
    // Other seasons
    else {
        glColor4f(1.0f, 0.9f, 0.0f, 0.0f);
    }

    // Drawing the glow circle using a triangle fan
    for (int i = 0; i <= triangleAmount; i++) {
        glVertex2f(
            cx + (r * 1.8f * cos(i * twicePi / triangleAmount)),
            cy + (r * 1.8f * sin(i * twicePi / triangleAmount)));
    }
    glEnd();

    // Sun Rays
    // Autumn case
    if (gameState == Autumn) {
        glColor4f(0.90f, 0.60f, 0.18f, 0.55f);
    }
    else {
        glColor4f(1.0f, 0.8f, 0.0f, 0.8f);
    }

    // Draw rays by rotating triangles around the center
    for (int i = 0; i < 360; i += 30) {
        glPushMatrix();
        glTranslatef(cx, cy, 0);
        glRotatef(i, 0, 0, 1);

        glBegin(GL_TRIANGLES);
        glVertex2f(r * 0.9f, -8); glVertex2f(r * 0.9f, 8); glVertex2f(r * 1.5f, 0);
        glEnd();
        glPopMatrix();
    }

    // Sun Disk
    // Autumn case
    if (gameState == Autumn) {
        glColor3f(0.93f, 0.74f, 0.25f);
    }
    // Other seasons
    else {
        glColor3f(1.0f, 0.95f, 0.2f);
    }

    drawCircle(cx, cy, r);
    // Summer case details
    if (gameState == Summer) {
        // Sunglasses
        glColor3f(0.0f, 0.0f, 0.0f);
        drawCircle(cx - r * 0.35f, cy + r * 0.1f, r * 0.25f); // Left
        drawCircle(cx + r * 0.35f, cy + r * 0.1f, r * 0.25f); // Right
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2f(cx - r * 0.1f, cy + r * 0.1f); glVertex2f(cx + r * 0.1f, cy + r * 0.1f);
        glEnd();
        // Cute Smile
        glBegin(GL_LINE_STRIP);
        glVertex2f(cx - r * 0.3f, cy - r * 0.25f); glVertex2f(cx - r * 0.15f, cy - r * 0.4f); glVertex2f(cx, cy - r * 0.45f); glVertex2f(cx + r * 0.15f, cy - r * 0.4f); glVertex2f(cx + r * 0.3f, cy - r * 0.25f);
        glEnd();
    }
    glPopMatrix(); // End of full sun rotation
}

// Clouds
void drawCloud(float x, float y, float size) {
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(x, y, size * 0.7f); drawCircle(x + size * 0.6f, y + size * 0.3f, size); drawCircle(x + size * 1.2f, y + size * 0.1f, size * 0.8f); drawCircle(x + size * 1.7f, y - size * 0.1f, size * 0.6f);
    // Rectangle to merge the circles
    glBegin(GL_QUADS);
    glVertex2f(x - size * 0.2f, y - size * 0.3f); glVertex2f(x + size * 2.0f, y - size * 0.3f); glVertex2f(x + size * 2.0f, y + size * 0.1f); glVertex2f(x - size * 0.2f, y + size * 0.1f);
    glEnd();
}
// NEW: Alias for cloud drawing used in Spring scene.
void drawStylizedCloud(float x, float y, float size) {
    drawCloud(x, y, size);
}

// Sea waves
void drawWaves() {
    float baseY = 235;
    glColor3f(0.0f, 0.55f, 0.75f);
    glBegin(GL_POLYGON);
    glVertex2f(0, baseY); glVertex2f(windowWidth, baseY); glVertex2f(windowWidth, baseY - 60); glVertex2f(0, baseY - 60);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= windowWidth; i += 20) {
        float waveY = baseY + ((i % 40 < 20) ? 8 : 0);
        glVertex2f(i, waveY); glVertex2f(i, waveY - 5);
    }
    glEnd();
}

// Red flower
void drawRedFlower(float x, float y, float size) {
    // Stem
    glColor3f(0.0f, 0.8f, 0.0f);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2f(x, y); glVertex2f(x, y + size * 2);
    glEnd();
    // Petals
    glColor3f(1.0f, 0.0f, 0.0f);
    float petalDist = size * 0.8f;
    drawCircle(x - petalDist, y + size * 2, size * 0.8f); drawCircle(x + petalDist, y + size * 2, size * 0.8f); drawCircle(x, y + size * 2 + petalDist, size * 0.8f); drawCircle(x, y + size * 2 - petalDist, size * 0.8f);
    // Flower center
    glColor3f(1.0f, 1.0f, 0.0f);
    drawCircle(x, y + size * 2, size * 0.5f);
}

// Birds in sky
void drawBird(float x, float y, float size) {
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x - size, y + size / 2); glVertex2f(x, y); glVertex2f(x + size, y + size / 2);
    glEnd();
}

// Sea shells
void drawShell(float x, float y, float size) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    // Shell body using a partial triangle fan
    glColor3f(1.0f, 0.95f, 0.85f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0); // Bottom center
    for (int i = 0; i <= 10; i++) {
        float angle = (i * PI / 10.0f);
        glVertex2f(size * cos(angle), size * sin(angle));
    }
    glEnd();
    // Shell detail lines
    glColor3f(0.85f, 0.75f, 0.65f);
    glLineWidth(1.5);
    glBegin(GL_LINES);
    for (int i = 1; i < 10; i++) {
        float angle = (i * PI / 10.0f);
        glVertex2f(0, 0);
        glVertex2f(size * cos(angle), size * sin(angle));
    }
    glEnd();
    glPopMatrix();
}

// Starfish
void drawStarfish(float x, float y, float radius) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    Rotate(20.0, 0, 0, 1);
    // Star body
    glColor3f(1.0f, 0.5f, 0.3f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 10; i++) {
        float r = (i % 2 == 0) ? radius : radius * 0.45f;
        float angle = i * 2.0f * PI / 10.0f;
        glVertex2f(r * cos(angle), r * sin(angle));
    }
    glEnd();
    // Detail points
    glColor3f(0.8f, 0.3f, 0.1f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 5; i++) {
        float angle = i * 2.0f * PI / 5.0f;
        glVertex2f(radius * 0.5f * cos(angle), radius * 0.5f * sin(angle));
    }
    glEnd();
    glPopMatrix();
}

// Sand toys
void drawSandToy(float x, float y, float scale) {
    // 1. Bucket
    // Bucket body
    glColor3f(0.0f, 0.4f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x + 40 * scale, y); glVertex2f(x + 50 * scale, y + 50 * scale); glVertex2f(x - 10 * scale, y + 50 * scale);
    glEnd();
    // Bucket rim
    glColor3f(0.0f, 0.3f, 0.7f);
    drawEllipse(x + 20 * scale, y + 50 * scale, 30 * scale, 8 * scale);
    // Handle
    glColor3f(0.2f, 0.2f, 0.2f);
    glLineWidth(2.0);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 10; i++) {
        float angle = i * PI / 10.0f;
        glVertex2f(x + 20 * scale + 30 * scale * cos(angle),
            y + 50 * scale + 25 * scale * sin(angle));
    }
    glEnd();
    // 2. Shovel
    float sx = x + 60 * scale;
    glColor3f(1.0f, 0.8f, 0.0f);
    // Shovel handle
    glBegin(GL_QUADS);
    glVertex2f(sx, y); glVertex2f(sx + 8 * scale, y); glVertex2f(sx + 8 * scale, y + 40 * scale); glVertex2f(sx, y + 40 * scale);
    glEnd();
    // Shovel head
    glBegin(GL_POLYGON);
    glVertex2f(sx - 10 * scale, y); glVertex2f(sx + 18 * scale, y); glVertex2f(sx + 15 * scale, y - 15 * scale); glVertex2f(sx - 7 * scale, y - 15 * scale);
    glEnd();
}

// Hills
void drawHills() {
    // Autumn case
    if (gameState == Autumn) {
        glColor3f(0.45f, 0.28f, 0.10f);
    }
    // Winter case
    else if (gameState == Winter) {
        // Far hill
        glColor3f(0.85f, 0.9f, 0.95f);
    }
    // Other seasons
    else {
        glColor3f(0.2, 0.5, 0.2);
    }

    glBegin(GL_POLYGON);
    // Creating hill using a circular polygon
    for (int i = 0; i <= triangleAmount; i++) {
        float angle = i * twicePi / triangleAmount;
        glVertex2f(200 + cos(angle) * 300,
            100 + sin(angle) * 150);
    }
    glEnd();

    // Near hill
    // Autumn case
    if (gameState == Autumn) {
        glColor3f(0.62f, 0.38f, 0.14f);
    }
    // Winter case
    else if (gameState == Winter) {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    // Other seasons
    else {
        glColor3f(0.3, 0.7, 0.3);
    }

    glBegin(GL_POLYGON);
    for (int i = 0; i <= triangleAmount; i++) {
        float angle = i * twicePi / triangleAmount;
        glVertex2f(700 + cos(angle) * 400,
            50 + sin(angle) * 200);
    }
    glEnd();
}

// Autumn tree
void drawAutumnTree(float x, float y, float scale) {
    glColor3f(0.38f, 0.20f, 0.06f);
    glBegin(GL_QUADS);
    glVertex2f(x - 5 * scale, y); glVertex2f(x + 5 * scale, y); glVertex2f(x + 5 * scale, y + 40 * scale);  glVertex2f(x - 5 * scale, y + 40 * scale);
    glEnd();

    glColor3f(0.76f, 0.45f, 0.17f);
    drawCircle(x, y + 52 * scale, 25 * scale);

    glColor3f(0.85f, 0.32f, 0.05f);
    drawCircle(x - 15 * scale, y + 40 * scale, 20 * scale);

    glColor3f(0.60f, 0.22f, 0.05f);
    drawCircle(x + 15 * scale, y + 40 * scale, 20 * scale);

    glColor3f(0.92f, 0.60f, 0.12f);
    drawCircle(x, y + 32 * scale, 18 * scale);
}

// Autumn leaf
void drawAutumnLeaf(float x, float y, float size, float r, float g, float b, float angle) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(30, 0, 0, 1);

    glColor3f(r, g, b);
    glBegin(GL_POLYGON);
    glVertex2f(0, size * 1.8f); glVertex2f(size * 1.2f, size * 0.4f); glVertex2f(size * 0.7f, -size * 1.5f); glVertex2f(0, -size * 2.0f); glVertex2f(-size * 0.7f, -size * 1.5f); glVertex2f(-size * 1.2f, size * 0.4f);
    glEnd();

    glColor3f(0.35f, 0.15f, 0.03f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(0, size * 1.8f); glVertex2f(0, -size * 2.0f);
    glEnd();

    glPopMatrix();
}

// Mountain for Winter scene
void drawMountain(float x, float y, float scale) {
    // Mountain base
    glColor3f(0.35f, 0.2f, 0.1f);
    glBegin(GL_POLYGON);
    glVertex2f(x - 250 * scale, y); glVertex2f(x + 250 * scale, y); glVertex2f(x + 100 * scale, y + 150 * scale); glVertex2f(x - 100 * scale, y + 150 * scale);
    glEnd();
    // Main mountain body (large triangle)
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 200 * scale, y + 50 * scale); glVertex2f(x + 200 * scale, y + 50 * scale); glVertex2f(x, y + 300 * scale);
    glEnd();
    // Mountain peak (snow cap)
    glColor3f(0.9f, 0.92f, 0.95f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 80 * scale, y + 180 * scale); glVertex2f(x + 80 * scale, y + 180 * scale); glVertex2f(x, y + 300 * scale);
    glEnd();

    // Snow accumulations
    glBegin(GL_POLYGON);
    glVertex2f(x - 80 * scale, y + 180 * scale); glVertex2f(x - 50 * scale, y + 160 * scale); glVertex2f(x, y + 175 * scale); glVertex2f(x + 50 * scale, y + 160 * scale); glVertex2f(x + 80 * scale, y + 180 * scale);
    glEnd();
}

// Snowman
void drawSnowman(float x, float y, float scale) {
    // 1. Body (three snowballs)
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(x, y, 40 * scale); drawCircle(x, y + 55 * scale, 30 * scale); drawCircle(x, y + 95 * scale, 20 * scale);
    // 2. Eyes and nose
    glColor3f(0.0f, 0.0f, 0.0f);
    drawCircle(x - 7 * scale, y + 100 * scale, 2.5 * scale); // Left eye
    drawCircle(x + 7 * scale, y + 100 * scale, 2.5 * scale); // Right eye
    // Nose
    glColor3f(1.0f, 0.5f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 95 * scale); glVertex2f(x, y + 91 * scale); glVertex2f(x + 15 * scale, y + 93 * scale);
    glEnd();
    // 3. Hands (branches)
    glColor3f(0.4f, 0.2f, 0.1f);
    glLineWidth(3);
    glBegin(GL_LINES);
    // Left arm
    glVertex2f(x - 25 * scale, y + 60 * scale); glVertex2f(x - 50 * scale, y + 85 * scale);
    // Right arm
    glVertex2f(x + 25 * scale, y + 60 * scale); glVertex2f(x + 50 * scale, y + 85 * scale);
    glEnd();

    // 4. Buttons
    glColor3f(0.0f, 0.0f, 0.0f);
    drawCircle(x, y + 65 * scale, 2.5 * scale); drawCircle(x, y + 55 * scale, 2.5 * scale); drawCircle(x, y + 45 * scale, 2.5 * scale);
}

// Winter tree
void drawWinterTree(float x, float y, float scale) {
    // 1. Trunk
    glColor3f(0.4f, 0.2f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(x - 5 * scale, y); glVertex2f(x + 5 * scale, y); glVertex2f(x + 5 * scale, y + 35 * scale); glVertex2f(x - 5 * scale, y + 35 * scale);
    glEnd();
    // 2. Green branches
    glColor3f(0.0f, 0.45f, 0.0f);
    // Bottom triangle
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 45 * scale, y + 30 * scale); glVertex2f(x + 45 * scale, y + 30 * scale); glVertex2f(x, y + 80 * scale);
    glEnd();
    // Top triangle
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 35 * scale, y + 60 * scale); glVertex2f(x + 35 * scale, y + 60 * scale); glVertex2f(x, y + 110 * scale);
    glEnd();

    // 3. Snow on tree
    glColor3f(1.0f, 1.0f, 1.0f);
    // Bottom snow
    glBegin(GL_POLYGON);
    glVertex2f(x - 38 * scale, y + 38 * scale); glVertex2f(x + 38 * scale, y + 38 * scale); glVertex2f(x + 10 * scale, y + 65 * scale); glVertex2f(x, y + 78 * scale); glVertex2f(x - 10 * scale, y + 65 * scale);
    glEnd();
    // Top snow
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 25 * scale, y + 75 * scale); glVertex2f(x + 25 * scale, y + 75 * scale); glVertex2f(x, y + 110 * scale);
    glEnd();
}

// Fancy Bee
void drawFancyBee(float x, float y, float scale) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f); // Overall scale control
    // 1. Wings
    glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
    drawCircle(-6, 8, 7); // Left wing
    drawCircle(6, 8, 7);  // Right wing
    // 2. Body
    glColor3f(1.0f, 0.9f, 0.1f);
    drawCircle(0, 0, 12);
    // 3. Black stripes
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(-10, 2);  glVertex2f(10, 2); glVertex2f(10, 4);   glVertex2f(-10, 4); glVertex2f(-11, -4); glVertex2f(11, -4); glVertex2f(11, -2);  glVertex2f(-11, -2);
    glEnd();
    // 4. Head
    drawCircle(10, 1, 6);
    // 5. Eye
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(12, 3, 1.5);
    glPopMatrix();
}


////
//UI//
////

// 1. Function to draw rounded rectangles for UI elements
void drawRoundedButton(float x, float y, float w, float h, float r, float g, float b, const char* text) {
    float radius = h / 2;
    glColor3f(r, g, b); // Button color
    drawCircle(x + radius, y + radius, radius); drawCircle(x + w - radius, y + radius, radius);
    glBegin(GL_QUADS);
    glVertex2f(x + radius, y); glVertex2f(x + w - radius, y); glVertex2f(x + w - radius, y + h); glVertex2f(x + radius, y + h);
    glEnd();
    // Button text 
    glColor3f(0, 0, 0);
    drawText(x + radius, y + h / 3, text);
}

// 2. Function to assemble the game interface (Question box + 4 choice buttons)
void drawGameUI() {
    // Question box (Light gray at top)
    drawRoundedButton(337, 520, 350, 50, 0.9f, 0.9f, 0.9f, " Look closely.. What season is this?");
    // Four choice buttons at the bottom
    float btnW = 120; float btnH = 40; float spacing = 20;
    float startX = (windowWidth - (4 * btnW + 3 * spacing)) / 2;
    drawRoundedButton(startX, 40, btnW, btnH, 1.0f, 1.0f, 1.0f, "  Winter"); // White
    drawRoundedButton(startX + (btnW + spacing), 40, btnW, btnH, 0.5f, 1.0f, 0.5f, "  Spring"); // Green
    drawRoundedButton(startX + 2 * (btnW + spacing), 40, btnW, btnH, 1.0f, 1.0f, 0.0f, "Summer"); // Yellow
    drawRoundedButton(startX + 3 * (btnW + spacing), 40, btnW, btnH, 1.0f, 0.6f, 0.2f, " Autumn"); // Orange
    // Feedback message (if answer is wrong)
    glColor3f(1, 0, 0);
    drawText(windowWidth / 2 - 80, 100, feedbackMessage);
}


////////////////////////
//Interaction and control//
 ////////////////////////

// NEW: Applies 2D zoom/viewing transformation to the whole scene.
void applyZoom() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Scale(zoom, zoom, 1.0);
}

// Handling arrow keys for movement and trophy rotation
void specialKeys(int key, int x, int y) {
    float step = 10.0f; // Movement speed
    switch (key) {
    case GLUT_KEY_LEFT:
        // Rotate trophy in end screen
        if (gameState == End_Screen) {
            trophyRotation -= 5.0f;
        }
        // Original logic: move left if not in Spring and within screen bounds
        else if (gameState != Spring && (300.0f + charX - step) > 50.0f) {
            charX -= step;
        }
        break;

    case GLUT_KEY_RIGHT:
        // Rotate trophy in end screen
        if (gameState == End_Screen) {
            trophyRotation += 5.0f;
        }
        // Move right logic
        else if (gameState != Spring && (300.0f + charX + step) < (windowWidth - 50.0f)) {
            charX += step;
        }
        break;

    case GLUT_KEY_UP:
        // Move up in Spring scene only
        if (gameState == Spring && (350.0f + charY + step) < (windowHeight - 50.0f)) {
            charY += step;
        }
        break;

    case GLUT_KEY_DOWN:
        // Move down in Spring scene only
        if (gameState == Spring && (350.0f + charY - step) > 120.0f) {
            charY -= step;
        }
        break;
    }
    glutPostRedisplay();
}


// NEW: Keyboard zoom control for 2D viewing (+/- keys).
void keyboard(unsigned char key, int x, int y) {
    if (key == '+') zoom += 0.1f;
    else if (key == '-' && zoom > 0.3f) zoom -= 0.1f;
    glutPostRedisplay();
}

// Mouse click handling for menu buttons and game choices
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // 1. Correct Y coordinates (OpenGL origin is bottom-left, Mouse is top-left)
        y = windowHeight - y;

        // 2. Start Screen handling
        if (gameState == 0) {
            // Check if Start button is clicked
            if (x >= (windowWidth / 2 - 110) && x <= (windowWidth / 2 + 110) &&
                y >= 175 && y <= 240) {
                gameState = Winter; // Start with Winter

#ifdef _WIN32
                // Change music to winter theme
                PlaySound(TEXT("assets\\audio\\winter.wav"), NULL, SND_ASYNC | SND_LOOP);
#endif
            }
        }

        // 3. Seasonal Screens logic (1 to 4)
        else if (gameState >= 1 && gameState <= 4) {
            float btnW = 120;
            float spacing = 20;
            float startX = (windowWidth - (4 * btnW + 3 * spacing)) / 2;

            for (int i = 0; i < 4; i++) {
                float bx = startX + i * (btnW + spacing);
                float by = 40;
                float bh = 40;

                // Check button click
                if (x >= bx && x <= bx + btnW && y >= by && y <= by + bh) {

                    if (i + 1 == gameState) {
                        // Correct Answer!
                        gameState++;
                        feedbackMessage = "Correct! Well done.";

                        // Change audio theme for each season
#ifdef _WIN32
                        if (gameState == Spring)
                            PlaySound(TEXT("assets\\audio\\spring.wav"), NULL, SND_ASYNC | SND_LOOP);
                        else if (gameState == Summer)
                            PlaySound(TEXT("assets\\audio\\summer.wav"), NULL, SND_ASYNC | SND_LOOP);
                        else if (gameState == Autumn)
                            PlaySound(TEXT("assets\\audio\\autumn.wav"), NULL, SND_ASYNC | SND_LOOP);
                        else if (gameState == End_Screen)
                            PlaySound(TEXT("assets\\audio\\end.wav"), NULL, SND_ASYNC | SND_ASYNC);
#endif

                        if (gameState > 4) {
                            gameState = 5;
                        }
                    }
                    else {
                        // Wrong Answer
                        feedbackMessage = "Ohh Wrong! Try again.";
                    }
                }
            }
        }
    }
    glutPostRedisplay();
}

////////
//screens//
 ////////

// Start Screen
void drawStartScreen() {
    // 1. Sky Blue background 
    glClearColor(0.85f, 0.95f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 2. Rainbow drawing using overlapping circles
    float centerX = windowWidth / 2;
    float centerY = 50;
    // Draw circles from outer to inner
    glColor3f(1.0f, 0.7f, 0.7f);  drawCircle(centerX, centerY, 550); // Arch 1
    glColor3f(1.0f, 0.85f, 0.6f); drawCircle(centerX, centerY, 525); // Arch 2
    glColor3f(1.0f, 1.0f, 0.7f);  drawCircle(centerX, centerY, 500); // Arch 3
    glColor3f(0.7f, 1.0f, 0.7f);  drawCircle(centerX, centerY, 475); // Arch 4
    glColor3f(0.7f, 0.9f, 1.0f);  drawCircle(centerX, centerY, 450); // Arch 5
    glColor3f(0.8f, 0.7f, 1.0f);  drawCircle(centerX, centerY, 425); // Arch 6

    // Void circle (background color) to create rainbow shape
    glColor3f(0.85f, 0.95f, 1.0f);
    drawCircle(centerX, centerY, 400);

    // 3. Rectangle to hide bottom part
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(windowWidth, 0); glVertex2f(windowWidth, 100); glVertex2f(0, 100);
    glEnd();

    // 4. Clouds
    drawCloud(100, 500, 35); drawCloud(350, 530, 40); drawCloud(650, 520, 38); drawCloud(900, 490, 35); drawCloud(500, 550, 30);

    // 5. Texts
    glColor3f(0.1f, 0.5f, 0.1f);
    drawText(windowWidth / 2 - 160, 440, "THE MAGIC SEASONS ADVENTURE");
    glColor3f(0.5f, 0.0f, 0.7f);
    drawText(windowWidth / 2 - 220, 400, "Move the character with arrows to discover the seasons!");

    // Flowers
    drawRedFlower(50, 35, 8); drawRedFlower(120, 20, 5); drawRedFlower(220, 45, 9); drawRedFlower(320, 15, 6);
    drawRedFlower(420, 50, 7); drawRedFlower(620, 45, 8); drawRedFlower(750, 20, 6); drawRedFlower(880, 55, 9); drawRedFlower(970, 30, 7);

    // Bees
    drawFancyBee(220, 330, 0.7f); drawFancyBee(800, 360, 0.6f); drawFancyBee(480, 480, 0.5f);
    drawFancyBee(180, 310, 0.9f); drawFancyBee(850, 380, 0.85f);

    // 6. Double Start Button (Shadow + Main)
    float bX = windowWidth / 2 - 110;
    float bY = 175;
    float bW = 220;
    float bH = 65;
    drawRoundedButton(bX + 5, bY - 5, bW, bH, 0.7f, 0.7f, 0.8f, "");
    drawRoundedButton(bX, bY, bW, bH, 1.0f, 0.5f, 0.7f, "     PLAY NOW");

    // 7. Instructions text
    glColor3f(0.4f, 0.5f, 0.6f);
    drawText(windowWidth / 2 - 100, 145, "Click the button to start");

    // 8. Character
    MyMainObj();
}

// Winter Scene
void drawWinterScene()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, winterTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0); glTexCoord2f(1.0f, 0.0f); glVertex2f(windowWidth, 0); glTexCoord2f(1.0f, 1.0f); glVertex2f(windowWidth, windowHeight); glTexCoord2f(0.0f, 1.0f); glVertex2f(0, windowHeight);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    drawMountain(400, 200, 1.0);
    drawHills();
    drawWinterTree(600, 200, 1.2f); drawWinterTree(850, 180, 1.0f);
    drawSnowman(150, 120, 1.8f);
    MyMainObj();
}

// NEW: Spring outfit uses two textures: floral shirt and pink skirt.
// ----------------------------- Spring Clothes -----------------------------
void drawTexturedTorsoSpring() {
    if (spring1Texture != 0) {
        glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, spring1Texture); C3(1.0, 1.0, 1.0);
        glBegin(GL_POLYGON); T2(0.0, 1.0); V2(-0.35, -0.60); T2(1.0, 1.0); V2(0.35, -0.60); T2(1.0, 0.0); V2(0.30, -1.25); T2(0.0, 0.0); V2(-0.30, -1.25); glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    else { C3(1.0, 0.92, 0.88); glBegin(GL_POLYGON); V2(-0.35, -0.60); V2(0.35, -0.60); V2(0.30, -1.25); V2(-0.30, -1.25); glEnd(); }
}

void drawTexturedSkirtSpring() {
    if (spring2Texture != 0) {
        glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, spring2Texture); C3(1.0, 1.0, 1.0);
        glBegin(GL_POLYGON); T2(0.0, 1.0); V2(-0.35, -1.25); T2(1.0, 1.0); V2(0.35, -1.25); T2(1.0, 0.0); V2(0.55, -1.95); T2(0.0, 0.0); V2(-0.55, -1.95); glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    else { C3(1.0, 0.45, 0.65); glBegin(GL_POLYGON); V2(-0.35, -1.25); V2(0.35, -1.25); V2(0.55, -1.95); V2(-0.55, -1.95); glEnd(); }

    C3(0.05, 0.05, 0.05); glLineWidth(1.2f);
    glBegin(GL_LINES); V2(-0.15, -1.25); V2(-0.25, -1.95); V2(0.0, -1.25); V2(0.0, -1.95); V2(0.15, -1.25); V2(0.25, -1.95); glEnd();
}

void drawSpringArms() {
    if (spring1Texture != 0) {
        glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, spring1Texture); C3(1.0, 1.0, 1.0);
        glBegin(GL_POLYGON); T2(0.0, 1.0); V2(-0.35, -0.60); T2(1.0, 1.0); V2(-0.52, -0.70); T2(1.0, 0.0); V2(-0.52, -1.05); T2(0.0, 0.0); V2(-0.30, -1.05); glEnd();
        glBegin(GL_POLYGON); T2(0.0, 1.0); V2(0.35, -0.60); T2(1.0, 1.0); V2(0.52, -0.70); T2(1.0, 0.0); V2(0.52, -1.05); T2(0.0, 0.0); V2(0.30, -1.05); glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    else {
        C3(1.0, 0.92, 0.88); glBegin(GL_POLYGON); V2(-0.35, -0.60); V2(-0.52, -0.70); V2(-0.52, -1.05); V2(-0.30, -1.05); glEnd();
        glBegin(GL_POLYGON); V2(0.35, -0.60); V2(0.52, -0.70); V2(0.52, -1.05); V2(0.30, -1.05); glEnd();
    }

    C3(1.0, 0.9, 0.8);
    glBegin(GL_POLYGON); V2(-0.35, -1.05); V2(-0.48, -1.05); V2(-0.48, -1.50); V2(-0.35, -1.50); glEnd();
    glBegin(GL_POLYGON); V2(0.35, -1.05); V2(0.48, -1.05); V2(0.48, -1.50); V2(0.35, -1.50); glEnd();
}

// NEW: Spring version of the character with textured clothes.
void MySpringObj() {
    glPushMatrix(); Translate(345.0 + charX, 300.0 + charY, 0.0); Scale(65.0, 65.0, 1.0);
    DrawBackHair(); DrawEarLeft(); DrawEarRight(); DrawFace(); DrawTopHairSimple(); DrawBangs(); DrawEyes(); DrawEyebrows(); DrawCheeks(); DrawSmile();
    drawNeck(); drawTexturedTorsoSpring(); drawSpringArms(); drawLegs(); drawTexturedSkirtSpring(); drawFeet();
    glPopMatrix();
}

// Drawing simple trees with layered circles for depth
void drawDaisy(double x, double y, double s);
void drawSimpleTree(double x, double y, double scale) {

    // trunk
    C3(0.45, 0.26, 0.08);
    glBegin(GL_QUADS);
    V2(x - 5.0 * scale, y);
    V2(x + 5.0 * scale, y);
    V2(x + 5.0 * scale, y + 40.0 * scale);
    V2(x - 5.0 * scale, y + 40.0 * scale);
    glEnd();

    // base shadow (Very dark)
    C3(0.04, 0.28, 0.18);
    drawCircle(x, y + 48.0 * scale, 26.0 * scale);
    drawCircle(x - 18.0 * scale, y + 38.0 * scale, 20.0 * scale);
    drawCircle(x + 18.0 * scale, y + 38.0 * scale, 20.0 * scale);

    // main leaves
    C3(0.10, 0.45, 0.25);
    drawCircle(x, y + 55.0 * scale, 23.0 * scale);
    drawCircle(x - 14.0 * scale, y + 48.0 * scale, 18.0 * scale);
    drawCircle(x + 14.0 * scale, y + 48.0 * scale, 18.0 * scale);

    // light highlights
    C3(0.30, 0.70, 0.30);
    drawCircle(x - 6.0 * scale, y + 60.0 * scale, 10.0 * scale);
    drawCircle(x + 10.0 * scale, y + 62.0 * scale, 11.0 * scale);
    drawCircle(x + 5.0 * scale, y + 45.0 * scale, 9.0 * scale);

    // white daisies on tree
    drawDaisy(x - 10.0 * scale, y + 60.0 * scale, 0.30 * scale);
    drawDaisy(x + 12.0 * scale, y + 55.0 * scale, 0.28 * scale);
}

void drawFlower(double x, double y, double size) {
    C3(0.0, 0.8, 0.0); glLineWidth(2.0f); glBegin(GL_LINES); V2(x, y); V2(x, y + size * 2.0); glEnd();
    C3(1.0, 0.0, 0.0); double petalDist = size * 0.8;
    drawCircle(x - petalDist, y + size * 2.0, size * 0.8); drawCircle(x + petalDist, y + size * 2.0, size * 0.8);
    drawCircle(x, y + size * 2.0 + petalDist, size * 0.8); drawCircle(x, y + size * 2.0 - petalDist, size * 0.8);
    C3(1.0, 1.0, 0.0); drawCircle(x, y + size * 2.0, size * 0.5);
}

// NEW: White daisy flower used on hills, bushes, and trees.
void drawDaisy(double x, double y, double s) {
    C3(0.18, 0.55, 0.18); glLineWidth(1.5f); glBegin(GL_LINES); V2(x, y); V2(x, y + 12.0 * s); glEnd();
    C3(1.0, 1.0, 1.0);
    for (int a = 0; a < 360; a += 45) {
        double rad = a * PI / 180.0;
        drawEllipse(x + cos(rad) * 5.5 * s, y + 12.0 * s + sin(rad) * 5.5 * s, 3.8 * s, 2.0 * s);
    }
    C3(1.0, 0.85, 0.1); drawCircle(x, y + 12.0 * s, 2.5 * s);
}

// NEW: Tree variation with daisies.
void drawFruitTree(double x, double y, double scale) {
    drawSimpleTree(x, y, scale);

    drawDaisy(x - 12.0 * scale, y + 53.0 * scale, 0.35 * scale);
    drawDaisy(x + 15.0 * scale, y + 48.0 * scale, 0.35 * scale);
    drawDaisy(x + 1.0 * scale, y + 65.0 * scale, 0.32 * scale);
}


// Drawing spring hills and placing daisies on them
void drawBaseHills() {
    C3(0.34, 0.76, 0.28);
    glBegin(GL_POLYGON);
    V2(0.0, 135.0); V2(0.0, 0.0); V2(450.0, 0.0); V2(500.0, 110.0);
    for (int i = 0; i <= triangleAmount; i++) {
        double angle = PI * static_cast<double>(i) / triangleAmount;
        V2(230.0 + cos(angle) * 330.0, 140.0 + sin(angle) * 92.0);
    }
    glEnd();

    C3(0.30, 0.70, 0.25);
    glBegin(GL_POLYGON);
    V2(560.0, 95.0); V2(1040.0, 95.0); V2(1040.0, 0.0); V2(520.0, 0.0);
    for (int i = 0; i <= triangleAmount; i++) {
        double angle = PI * static_cast<double>(i) / triangleAmount;
        V2(790.0 + cos(angle) * 315.0, 128.0 + sin(angle) * 88.0);
    }
    glEnd();

    drawDaisy(80.0, 150.0, 0.85);
    drawDaisy(125.0, 190.0, 0.75);
    drawDaisy(210.0, 165.0, 0.90);
    drawDaisy(355.0, 185.0, 0.80);
    drawDaisy(430.0, 145.0, 0.80);
    drawDaisy(35.0, 35.0, 0.80);
    drawDaisy(200.0, 40.0, 0.80);
    drawDaisy(800.0, 40.0, 0.80);
    drawDaisy(700.0, 20.0, 0.80);
    drawFruitTree(20.0, 70.0, 1.50);
    drawFruitTree(90.0, 30.0, 1.40);

}

// Drawing left side mountains with green peaks and snow tops
void drawLeftMountainWithTrees() {

    // First Mountain
    C3(0.20, 0.50, 0.28);
    glBegin(GL_TRIANGLES);
    V2(0.0, 200.0); V2(120.0, 420.0); V2(250.0, 200.0);
    glEnd();

    // Second Mountain
    C3(0.16, 0.38, 0.22);
    glBegin(GL_TRIANGLES);
    V2(200.0, 195.0); V2(280.0, 320.0); V2(360.0, 195.0);
    glEnd();

    // Snow caps
    C3(0.95, 0.98, 1.0);

    // Peak 1
    glBegin(GL_TRIANGLES);
    V2(87.0, 360.0);
    V2(120.0, 420.0);
    V2(156.0, 360.0);
    glEnd();

    // Peak 2
    glBegin(GL_TRIANGLES);
    V2(260.0, 290.0);
    V2(280.0, 320.0);
    V2(300.0, 290.0);
    glEnd();

    drawFruitTree(615.0, 150.0, 1.50);
    drawFruitTree(669.0, 140.0, 1.25);
    drawFruitTree(640.0, 70.0, 1.00);
    drawFruitTree(1000.0, 50.0, 1.10);
    drawFruitTree(950.0, 30.0, 1.00);
    drawFruitTree(100.0, 190.0, 1.40);
    drawFruitTree(150.0, 185.0, 1.20);
    drawFruitTree(200.0, 190.0, 1.40);
    drawFlower(145.0, 150.0, 5.0);
    drawFlower(190.0, 100.0, 5.0);
    drawFlower(260.0, 145.0, 4.8);
    drawFlower(390.0, 155.0, 4.5);
}

// Drawing river with texture
void drawLongRiverBesidePath() {
    if (spring3Texture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, spring3Texture);
        C3(1.0, 1.0, 1.0);

        glBegin(GL_POLYGON);
        glTexCoord2f(0.0f, 0.0f); V2(455.0, 0.0);
        glTexCoord2f(1.0f, 0.0f); V2(620.0, 0.0);
        glTexCoord2f(1.0f, 1.0f); V2(570.0, 215.0);
        glTexCoord2f(0.0f, 1.0f); V2(400.0, 230.0);

        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    else {
        C3(0.28, 0.65, 0.95);

        glBegin(GL_POLYGON);
        V2(455.0, 0.0);
        V2(600.0, 0.0);
        V2(585.0, 170.0);
        V2(565.0, 250.0);
        V2(510.0, 270.0);
        V2(455.0, 250.0);
        glEnd();
    }
}


// Drawing the dirt path for the character
void drawDirtPath() {
    C3(0.43, 0.26, 0.12);
    glBegin(GL_POLYGON); V2(250.0, 0.0); V2(455.0, 0.0); V2(405.0, 230.0); V2(315.0, 230.0); glEnd();
    C3(0.78, 0.64, 0.45);
    glBegin(GL_POLYGON); V2(275.0, 0.0); V2(428.0, 0.0); V2(390.0, 230.0); V2(330.0, 230.0); glEnd();
    C3(0.58, 0.40, 0.22); glLineWidth(2.0f);
    glBegin(GL_LINES); V2(300.0, 35.0); V2(415.0, 35.0); V2(292.0, 82.0); V2(410.0, 82.0); V2(315.0, 135.0); V2(398.0, 135.0); glEnd();
}

// Tulip drawing helper
void drawTulip(double x, double y, double size, double r, double g, double b) {
    C3(0.0, 0.55, 0.15); glLineWidth(2.0f);
    glBegin(GL_LINES); V2(x, y); V2(x, y + size * 3.2); glEnd();
    glBegin(GL_TRIANGLES);
    V2(x, y + size * 1.4); V2(x - size * 1.1, y + size * 2.0); V2(x, y + size * 2.1);
    V2(x, y + size * 1.6); V2(x + size * 1.1, y + size * 2.2); V2(x, y + size * 2.2);
    glEnd();
    C3(r, g, b); glBegin(GL_POLYGON);
    V2(x - size * 1.0, y + size * 3.0); V2(x - size * 0.7, y + size * 4.0); V2(x, y + size * 3.6);
    V2(x + size * 0.7, y + size * 4.0); V2(x + size * 1.0, y + size * 3.0); V2(x + size * 0.55, y + size * 2.4); V2(x - size * 0.55, y + size * 2.4);
    glEnd();
    C3(r * 0.85, g * 0.85, b * 0.85); glBegin(GL_TRIANGLES);
    V2(x - size * 0.8, y + size * 3.0); V2(x - size * 0.2, y + size * 4.35); V2(x + size * 0.1, y + size * 3.0);
    V2(x + size * 0.8, y + size * 3.0); V2(x + size * 0.2, y + size * 4.35); V2(x - size * 0.1, y + size * 3.0);
    glEnd();
}

// Garden fence
void drawWhiteFence(double x, double y, double w) {
    C3(1.0, 1.0, 1.0);
    for (double px = x; px <= x + w; px += 30.0) {
        glBegin(GL_QUADS); V2(px, y); V2(px + 8.0, y); V2(px + 8.0, y + 34.0); V2(px, y + 34.0); glEnd();
        glBegin(GL_TRIANGLES); V2(px - 2.0, y + 34.0); V2(px + 10.0, y + 34.0); V2(px + 4.0, y + 43.0); glEnd();
    }
    glBegin(GL_QUADS); V2(x - 12.0, y + 11.0); V2(x + w + 18.0, y + 11.0); V2(x + w + 18.0, y + 18.0); V2(x - 12.0, y + 18.0); glEnd();
    glBegin(GL_QUADS); V2(x - 12.0, y + 27.0); V2(x + w + 18.0, y + 27.0); V2(x + w + 18.0, y + 34.0); V2(x - 12.0, y + 34.0); glEnd();
}

// Creating a field of tulips
void drawTulipGarden() {
    for (int row = 0; row < 4; row++) {
        for (int i = 0; i < 16; i++) {

            double x = 700.0 + i * 20.0 + row * 4.0;
            double y = 80.0 + row * 26.0;

            double s = 8.0;

            if (row % 3 == 0)
                drawTulip(x, y, s, 1.0, 0.10, 0.18);
            else if (row % 3 == 1)
                drawTulip(x, y, s, 1.0, 0.45, 0.10);
            else
                drawTulip(x, y, s, 1.0, 0.75, 0.15);
        }
    }

    drawWhiteFence(585.0, 190.0, 390.0);
}

// Windmill for background decoration
void drawWindmill(double x, double y, double scale) {
    glPushMatrix();
    Translate(x, y, 0.0);
    Scale(scale, scale, 1.0);

    // Mill body
    C3(0.45, 0.28, 0.12); glBegin(GL_POLYGON); V2(-35.0, 0.0); V2(35.0, 0.0); V2(25.0, 95.0); V2(-25.0, 95.0); glEnd();
    C3(0.30, 0.18, 0.08); glBegin(GL_TRIANGLES); V2(-42.0, 95.0); V2(42.0, 95.0); V2(0.0, 135.0); glEnd();
    C3(0.15, 0.10, 0.05); glBegin(GL_POLYGON); V2(-10.0, 0.0); V2(10.0, 0.0); V2(10.0, 35.0); V2(-10.0, 35.0); glEnd();
    C3(0.9, 0.75, 0.45); drawCircle(0.0, 75.0, 6.0);

    // Rotating blades
    C3(0.55, 0.38, 0.18);
    for (int a = 0; a < 360; a += 90) {
        glPushMatrix();
        Translate(0.0, 75.0, 0.0);
        Rotate(static_cast<double>(a) + windmillAngle, 0.0, 0.0, 1.0);
        glBegin(GL_POLYGON); V2(0.0, -3.0); V2(75.0, -8.0); V2(80.0, 8.0); V2(0.0, 3.0); glEnd();
        glPopMatrix();
    }
    glPopMatrix();
}

// Animated butterflies with wing flapping logic
void drawButterfly(double x, double y, double s, double phase, double r, double g, double b) {
    double flap = sin(butterflyTime * 6.0 + phase) * 4.0;
    double bob = sin(butterflyTime * 2.0 + phase) * 8.0;

    glPushMatrix();
    Translate(x + sin(butterflyTime + phase) * 8.0, y + bob, 0.0);
    Scale(s, s, 1.0);

    C3(r, g, b);
    glPushMatrix(); Rotate(-20.0 + flap, 0.0, 0.0, 1.0); drawEllipse(-6.0, 3.0, 7.0, 4.0); glPopMatrix();
    glPushMatrix(); Rotate(20.0 - flap, 0.0, 0.0, 1.0); drawEllipse(6.0, 3.0, 7.0, 4.0); glPopMatrix();
    glPushMatrix(); Rotate(15.0 - flap, 0.0, 0.0, 1.0); drawEllipse(-5.0, -4.0, 5.0, 3.5); glPopMatrix();
    glPushMatrix(); Rotate(-15.0 + flap, 0.0, 0.0, 1.0); drawEllipse(5.0, -4.0, 5.0, 3.5); glPopMatrix();

    C3(0.10, 0.08, 0.05);
    drawEllipse(0.0, 0.0, 1.8, 6.0);
    glLineWidth(1.2f);
    glBegin(GL_LINES); V2(0.0, 5.0); V2(-5.0, 10.0); V2(0.0, 5.0); V2(5.0, 10.0); glEnd();

    glPopMatrix();
}


void drawButterflies() {
    // Three butterflies near the tulips.
    drawButterfly(665.0, 290.0, 1.05, 0.0, 1.0, 0.35, 0.75);
    drawButterfly(760.0, 210.0, 0.95, 1.2, 1.0, 0.75, 0.20);
    drawButterfly(900.0, 250.0, 1.00, 2.2, 0.60, 0.45, 1.0);

    // Five butterflies around the trees.
    drawButterfly(95.0, 190.0, 0.90, 0.5, 1.0, 0.65, 0.15);
    drawButterfly(150.0, 145.0, 0.85, 1.6, 0.75, 0.40, 1.0);
    drawButterfly(220.0, 80.0, 0.80, 2.6, 1.0, 0.45, 0.55);
    drawButterfly(259.0, 139.0, 0.88, 3.3, 0.30, 0.75, 1.0);
    drawButterfly(200.0, 130.0, 0.82, 4.0, 1.0, 0.85, 0.20);
}


// NEW: Timer function updates butterfly and sun animation every frame (16ms)
void update(int value) {
    butterflyTime += 0.035f;
    sunAngle += 0.4f;

    // Mill rotation update
    windmillAngle += 2.0f;
    if (windmillAngle >= 360.0f)
        windmillAngle -= 360.0f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// Drawing bushes with daisies
void drawBush(double x, double y, double s) {

    // Dark base shadow
    C3(0.04, 0.28, 0.18);
    drawCircle(x + 5.0 * s, y - 2.0 * s, 14.0 * s);
    drawCircle(x + 22.0 * s, y - 4.0 * s, 16.0 * s);
    drawCircle(x + 38.0 * s, y - 2.0 * s, 14.0 * s);

    // Main bush body
    C3(0.10, 0.45, 0.25);
    drawCircle(x + 2.0 * s, y + 8.0 * s, 13.0 * s);
    drawCircle(x + 15.0 * s, y + 15.0 * s, 17.0 * s);
    drawCircle(x + 32.0 * s, y + 14.0 * s, 18.0 * s);
    drawCircle(x + 48.0 * s, y + 8.0 * s, 13.0 * s);

    // Medium green patches
    C3(0.24, 0.62, 0.30);
    drawCircle(x + 12.0 * s, y + 20.0 * s, 9.0 * s);
    drawCircle(x + 28.0 * s, y + 23.0 * s, 10.0 * s);
    drawCircle(x + 42.0 * s, y + 17.0 * s, 8.0 * s);
    drawCircle(x + 20.0 * s, y + 7.0 * s, 8.0 * s);

    // Light highlights
    C3(0.55, 0.85, 0.28);
    drawCircle(x + 14.0 * s, y + 25.0 * s, 4.5 * s);
    drawCircle(x + 30.0 * s, y + 27.0 * s, 5.0 * s);
    drawCircle(x + 39.0 * s, y + 21.0 * s, 4.0 * s);
    drawCircle(x + 23.0 * s, y + 13.0 * s, 4.0 * s);

    // Daisies on bush
    drawDaisy(x + 10.0 * s, y + 30.0 * s, 0.28 * s);
    drawDaisy(x + 30.0 * s, y + 32.0 * s, 0.25 * s);
    drawDaisy(x + 43.0 * s, y + 24.0 * s, 0.22 * s);
}


// Spring Scene
void drawSpringScene() {
    // Sky background
    glBegin(GL_QUADS);
    C3(0.55, 0.82, 1.0); V2(0.0, 0.0);
    C3(0.55, 0.82, 1.0); V2(windowWidth, 0.0);
    C3(0.82, 0.95, 1.0); V2(windowWidth, windowHeight);
    C3(0.82, 0.95, 1.0); V2(0.0, windowHeight);
    glEnd();

    drawAttractiveSun(150.0, 480.0, 45.0);
    drawStylizedCloud(560.0, 525.0, 22.0);
    drawStylizedCloud(825.0, 485.0, 20.0);
    drawStylizedCloud(300.0, 470.0, 25.0);
    drawStylizedCloud(450.0, 430.0, 24.0);

    drawBaseHills();
    drawLongRiverBesidePath();

    drawDirtPath();

    // Bush borders around river and path
    drawBush(400.0, 225.0, 0.85);
    drawBush(403.0, 210.0, 0.85);
    drawBush(404.0, 180.0, 0.85);
    drawBush(405.0, 150.0, 0.85);
    drawBush(410.0, 120.0, 0.85);
    drawBush(415.0, 90.0, 0.85);
    drawBush(420.0, 60.0, 0.85);
    drawBush(425.0, 30.0, 0.85);
    drawBush(430.0, 0.0, 0.85);

    drawBush(530.0, 225.0, 0.85);
    drawBush(540.0, 210.0, 0.85);
    drawBush(550.0, 180.0, 0.85);
    drawBush(560.0, 150.0, 0.85);
    drawBush(570.0, 120.0, 0.85);
    drawBush(580.0, 90.0, 0.85);
    drawBush(590.0, 60.0, 0.85);
    drawBush(600.0, 30.0, 0.85);
    drawBush(610.0, 0.0, 0.85);


    drawTulipGarden();
    drawLeftMountainWithTrees();

    // Small bushes placement
    drawBush(20.0, 70.0, 0.40);
    drawBush(10.0, 70.0, 0.40);
    drawBush(90.0, 30.0, 0.40);
    drawBush(80.0, 30.0, 0.40);
    drawBush(615.0, 150.0, 0.40);
    drawBush(605.0, 150.0, 0.40);
    drawBush(669.0, 140.0, 0.40);
    drawBush(659.0, 140.0, 0.40);
    drawBush(640.0, 70.0, 0.40);
    drawBush(630.0, 70.0, 0.40);
    drawBush(1000.0, 50.0, 0.40);
    drawBush(990.0, 50.0, 0.40);
    drawBush(950.0, 30.0, 0.40);
    drawBush(940.0, 30.0, 0.40);
    drawBush(100.0, 190.0, 0.40);
    drawBush(90.0, 190.0, 0.40);
    drawBush(150.0, 185.0, 0.40);
    drawBush(140.0, 185.0, 0.40);
    drawBush(200.0, 190.0, 0.40);
    drawBush(190.0, 190.0, 0.40);


    drawButterflies();
    drawWindmill(980.0, 190.0, 1.20);

    MySpringObj();

    C3(0.05, 0.25, 0.25);
    drawText(455.0, 555.0, "   Spring");
}

// Summer Scene
void drawSummerScene()
{// Sky gradient
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.8f, 1.0f);
    glVertex2f(0, 0); glVertex2f(windowWidth, 0);
    glColor3f(0.6f, 0.9f, 1.0f);
    glVertex2f(windowWidth, 600); glVertex2f(0, 600);
    glEnd();
    // Birds
    drawBird(200, 500, 15); drawBird(250, 470, 12); drawBird(400, 530, 10);
    // Sun 
    drawAttractiveSun(800, 500, 80);
    // Sand (Beach)
    glColor3f(0.96f, 0.87f, 0.70f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(windowWidth, 0); glVertex2f(windowWidth, 230); glVertex2f(0, 230);
    glEnd();

    // Sea
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.6f, 0.8f); // Light blue near shore
    glVertex2f(0, 230); glVertex2f(windowWidth, 230);
    glColor3f(0.0f, 0.3f, 0.5f); // Dark blue at horizon
    glVertex2f(windowWidth, 450); glVertex2f(0, 450);
    glEnd();
    // Waves
    drawWaves();
    // Toys
    drawSandToy(500, 70, 0.9f);
    // Starfish
    drawStarfish(120, 140, 18); drawStarfish(380, 50, 22); drawStarfish(680, 130, 16); drawStarfish(920, 40, 20);
    // Shells
    drawShell(60, 70, 12); drawShell(200, 30, 15); drawShell(280, 160, 10); drawShell(440, 110, 11); drawShell(580, 20, 14); drawShell(630, 80, 12);
    drawShell(780, 140, 13); drawShell(840, 40, 15); drawShell(940, 110, 10); drawShell(970, 170, 9);
    // Small details
    glColor3f(1.0f, 1.0f, 0.9f);
    drawCircle(320, 90, 5); drawCircle(560, 150, 6); drawCircle(720, 60, 5); drawCircle(180, 175, 4);
    // Character
    MyMainObj();
}

// Autumn Scene
void drawAutumnScene()
{// Sky
    glBegin(GL_QUADS);
    glColor3f(0.78f, 0.86f, 0.92f); glVertex2f(0, 0);
    glColor3f(0.76f, 0.84f, 0.90f); glVertex2f(windowWidth, 0);
    glColor3f(0.88f, 0.92f, 0.95f); glVertex2f(windowWidth, 600);
    glColor3f(0.84f, 0.90f, 0.94f); glVertex2f(0, 600);
    glEnd();
    // Sun
    drawAttractiveSun(150, 480, 45);
    // Clouds
    drawCloud(50, 530, 25); drawCloud(150, 500, 30); drawCloud(80, 460, 20); drawCloud(700, 520, 35); drawCloud(850, 500, 25); drawCloud(650, 470, 20); drawCloud(450, 510, 15); drawCloud(550, 490, 20);
    // Hills
    drawHills();
    // Autumn trees
    drawAutumnTree(50, 210, 0.9f); drawAutumnTree(250, 230, 0.8f); drawAutumnTree(540, 220, 1.0f); drawAutumnTree(800, 230, 0.85f); drawAutumnTree(990, 180, 0.95f); drawAutumnTree(150, 140, 2.0f);
    drawAutumnTree(450, 110, 2.4f); drawAutumnTree(700, 125, 2.2f); drawAutumnTree(900, 100, 2.5f);
    // Autumn leaves in different colors and rotations
    // Yellow
    drawAutumnLeaf(80, 190, 4, 0.90f, 0.70f, 0.10f, 30); drawAutumnLeaf(220, 210, 3, 0.90f, 0.70f, 0.10f, -45); drawAutumnLeaf(380, 185, 4, 0.90f, 0.70f, 0.10f, 15); drawAutumnLeaf(500, 225, 3, 0.90f, 0.70f, 0.10f, 60); drawAutumnLeaf(620, 200, 4, 0.90f, 0.70f, 0.10f, -10); drawAutumnLeaf(880, 215, 4, 0.90f, 0.70f, 0.10f, 25);
    // Red
    drawAutumnLeaf(30, 160, 4, 0.70f, 0.10f, 0.00f, -20); drawAutumnLeaf(180, 145, 5, 0.70f, 0.10f, 0.00f, 40); drawAutumnLeaf(440, 155, 4, 0.70f, 0.10f, 0.00f, 90); drawAutumnLeaf(580, 135, 5, 0.70f, 0.10f, 0.00f, -15); drawAutumnLeaf(730, 170, 4, 0.70f, 0.10f, 0.00f, 55); drawAutumnLeaf(920, 150, 4, 0.70f, 0.10f, 0.00f, -60);
    // Orange
    drawAutumnLeaf(130, 80, 5, 0.85f, 0.30f, 0.05f, 110); drawAutumnLeaf(170, 90, 4, 0.85f, 0.30f, 0.05f, -10); drawAutumnLeaf(430, 75, 5, 0.85f, 0.30f, 0.05f, 20); drawAutumnLeaf(470, 85, 4, 0.85f, 0.30f, 0.05f, 150); drawAutumnLeaf(680, 90, 5, 0.85f, 0.30f, 0.05f, -35); drawAutumnLeaf(720, 80, 4, 0.85f, 0.30f, 0.05f, 45);
    drawAutumnLeaf(100, 25, 7, 0.85f, 0.30f, 0.05f, 0); drawAutumnLeaf(300, 15, 6, 0.85f, 0.30f, 0.05f, 80); drawAutumnLeaf(550, 30, 7, 0.85f, 0.30f, 0.05f, -25); drawAutumnLeaf(800, 20, 6, 0.85f, 0.30f, 0.05f, 15); drawAutumnLeaf(950, 35, 7, 0.85f, 0.30f, 0.05f, -50);
    // Yellow
    drawAutumnLeaf(50, 50, 5, 0.90f, 0.70f, 0.10f, 45); drawAutumnLeaf(200, 40, 6, 0.90f, 0.70f, 0.10f, -15); drawAutumnLeaf(400, 55, 5, 0.90f, 0.70f, 0.10f, 120); drawAutumnLeaf(650, 45, 6, 0.90f, 0.70f, 0.10f, 30); drawAutumnLeaf(850, 60, 5, 0.90f, 0.70f, 0.10f, -10); drawAutumnLeaf(1010, 50, 6, 0.90f, 0.70f, 0.10f, 20);
    // Red
    drawAutumnLeaf(150, 15, 5, 0.70f, 0.10f, 0.00f, -90); drawAutumnLeaf(350, 30, 4, 0.70f, 0.10f, 0.00f, 15); drawAutumnLeaf(600, 25, 5, 0.70f, 0.10f, 0.00f, 45); drawAutumnLeaf(750, 10, 6, 0.70f, 0.10f, 0.00f, -110); drawAutumnLeaf(900, 45, 5, 0.70f, 0.10f, 0.00f, 30);
    // Character
    MyMainObj();
}

// End Screen
void drawEndScreen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.85f, 0.95f, 1.0f, 1.0f);

    float centerX = windowWidth / 2;
    // --- 1. Rainbow background ---
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    float rainbowY = 100;
    glColor3f(1.0f, 0.7f, 0.7f);  drawCircle(centerX, rainbowY, 550);
    glColor3f(1.0f, 0.85f, 0.6f); drawCircle(centerX, rainbowY, 525);
    glColor3f(1.0f, 1.0f, 0.7f);  drawCircle(centerX, rainbowY, 500);
    glColor3f(0.7f, 1.0f, 0.7f);  drawCircle(centerX, rainbowY, 475);
    glColor3f(0.7f, 0.9f, 1.0f);  drawCircle(centerX, rainbowY, 450);
    glColor3f(0.8f, 0.7f, 1.0f);  drawCircle(centerX, rainbowY, 425);
    glColor3f(0.85f, 0.95f, 1.0f); drawCircle(centerX, rainbowY, 400);

    // --- 2. 3D Trophy
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_COLOR_MATERIAL);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(65, (float)windowWidth / windowHeight, 0.1, 50);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    gluLookAt(transValueX, transValueY, transValueZ, 0, 0, 0, 0, 1, 0);

    // Trophy container for rotation
    glPushMatrix();
    glRotatef(trophyRotation, 0, 1, 0);
    glColor3f(0.9f, 0.8f, 0.3f);
    // Trophy cup (Cone)
    glPushMatrix();
    glTranslatef(0, 0.1f, 0);
    glRotatef(-90, 1, 0, 0);
    glutSolidCone(0.75, 0.8, 30, 30);
    glPopMatrix();
    // Trophy stem (Cube)
    glPushMatrix();
    glTranslatef(0, -0.3f, 0);
    glScalef(0.25f, 0.6f, 0.25f);
    glutSolidCube(1.0);
    glPopMatrix();
    // Trophy base
    // Top base
    glPushMatrix();
    glTranslatef(0, -0.7f, 0);
    glScalef(0.9f, 0.2f, 0.9f);
    glutSolidCube(1.0);
    glPopMatrix();
    // Bottom base
    glPushMatrix();
    glTranslatef(0, -0.9f, 0);
    glScalef(1.3f, 0.2f, 1.3f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPopMatrix();

    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    glColor3f(0.3f, 0.0f, 0.5f);
    drawText(centerX - 100, 100, "CONGRATULATIONS!");
    glColor3f(0.1f, 0.5f, 0.1f);
    drawText(centerX - 85, 60, "YOU ARE A HERO");
}




void init() {
    glClearColor(1, 1, 1, 1);

    // Enable transparency for clouds and glow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    // Load all textures
    winterTexture = LoadTexture(winterPath, 880, 586); spring1Texture = LoadTexture(spring1Path, 512, 512); spring2Texture = LoadTexture(spring2Path, 512, 512); spring3Texture = LoadTexture(spring3Path, 512, 512); autumnTexture = LoadTexture(autumnPath, 256, 256);
}

// Main display function to switch between game states
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    applyZoom();

    // Constant rotation update for sun
    sunAngle += 0.1f;
    if (sunAngle > 360) sunAngle -= 360;

    if (gameState == 0) drawStartScreen();
    else if (gameState == 1) { drawWinterScene(); drawGameUI(); }
    else if (gameState == 2) { drawSpringScene(); drawGameUI(); }
    else if (gameState == 3) { drawSummerScene(); drawGameUI(); }
    else if (gameState == 4) { drawAutumnScene(); drawGameUI(); }
    else if (gameState == 5) drawEndScreen();

    glutPostRedisplay();
    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("GUESS THE SEASON GAME");

#ifdef _WIN32
    PlaySound(TEXT("assets\\audio\\start.wav"), NULL, SND_ASYNC | SND_LOOP);
#endif

    init();
    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMainLoop();
    return 0;
}