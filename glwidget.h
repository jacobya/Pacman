#ifndef __GLWIDGET__INCLUDE__
#define __GLWIDGET__INCLUDE__

#include <QGLWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMouseEvent>
#include <QTimer>
#include <glm/glm.hpp>

#define GLM_FORCE_RADIANS

using glm::mat3;
using glm::mat4;
using glm::vec3;
using glm::vec4;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT

    public:
        GLWidget(QWidget *parent=0);
        ~GLWidget();

        GLuint loadShaders(const char* vertf, const char* fragf);
    protected:
        void initializeGL();
        void resizeGL(int w, int h);
        void paintGL();

        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);
        void leftWall(mat4 transform);
        void leftMortar(mat4 transform);
        void rightWall(mat4 transform);
        void rightMortar(mat4 transform);
        void frontWall(mat4 transform);
        void frontMortar(mat4 transform);
        void backWall(mat4 transform);
        void backMortar(mat4 transform);
        //void renderBrick();
        void renderMortar(mat4 transform);
        void create(vec3 pts[]);
        void createWalls(mat4 transform);
        void brick(mat4 tran);
        vec4 CalculateVectorNormal(vec3 pts1, vec3 pts2,vec3 pts3);
        void mortar(mat4 tran);
        bool bound();
    // Part 2 - add an animate slot

public slots:
        void animate();

    private:
        void initializeBrick();
        void renderBrick(mat4 transform);

        GLuint mortarText;
        GLuint normalBuffer;

        GLuint brickProg;
        GLuint brickVao;
        GLint brickProjMatrixLoc;
        GLint brickViewMatrixLoc;
        GLint brickModelMatrixLoc;
        GLint brickAlphaLoc;
        GLuint textureObject4;
        GLuint bnormalBuffer;

        GLint brickLightLoc;
        GLint brickLightAmbLoc;
        GLint brickCameraLoc;
        GLint brickShineLoc;

        void initializeCube();
        void renderCube(mat4 transform);
        void renderCubeWall(mat4 transform);

        GLuint cubeProg;
        GLuint cubeVao;
        GLuint cnormalBuffer;
        GLint cubeProjMatrixLoc;
        GLint cubeViewMatrixLoc;
        GLint cubeModelMatrixLoc;
        GLuint textureObject;

        GLint cubeAlphaLoc;
        GLint cubeLightLoc;
        GLint cubeLightAmbLoc;
        GLint cubeCameraLoc;
        GLint cubeShineLoc;


        void initializeWall();
        void renderWall();


        GLuint wallcolorBuffer;
        GLuint wallnormalBuffer;
        GLuint wallProg;
        GLuint wallVao;
        GLint wallProjMatrixLoc;
        GLint wallViewMatrixLoc;
        GLint wallModelMatrixLoc;
        GLint wallTrackballMatrixLoc;
        GLint wallCameraLoc;
        GLuint textureObject3;

        GLint wallLightLoc;
        GLint wallLightAmbLoc;
        GLint wallLightDiffLoc;
        GLint wallShineLoc;

        void initializePlane();
        void renderPlane(mat4 transform);

        GLuint planeProg;
        GLuint planeVao;
        GLint planeProjMatrixLoc;
        GLint planeViewMatrixLoc;
        GLint planeModelMatrixLoc;
        GLuint textureObject2;


        GLint planeAlphaLoc;
        GLint planeLightLoc;
        GLint planeLightAmbLoc;
        GLint planeCameraLoc;
        GLint planeShineLoc;

        void initializeGrid();
        void renderGrid();

        GLuint gridProg;
        GLuint gridVao;
        GLint gridProjMatrixLoc;
        GLint gridViewMatrixLoc;
        GLint gridModelMatrixLoc;

        mat4 projMatrix;
        mat4 viewMatrix;
        mat4 modelMatrix;

        float yaw;
        float pitch;
        float widthScale;
        float heightScale;
        float depthScale;

        mat4 yawMatrix;
        mat4 pitchMatrix;
        mat4 orientation;

        // Part 2 - Add a QTimer variable for our render loop.

        QTimer *timer;

        vec3 position;
        vec3 velocity;
        //vec3 colors[];
        vec3 Mortar[];
        vec3 pt[24];

        vec4 normal[24];

        bool forward;
        bool back;
        bool left;
        bool right;
        bool up;
        bool down;
        bool fly;
        bool jump;
        bool jumpFlag;

        int numLayers;
        int width;
        int height;

        glm::vec2 lastPt;
        void updateView();
        char loc[54][54];
        void storeDots();

        QImage *bricks;
        QImage *gg;
        QImage *inky;
        QImage *blinky;
        QImage *pinky;
        QImage *clyde;

        //void renderCube(mat4 transform);
};

#endif
