#include "glwidget.h"
#include <iostream>
#include <QOpenGLTexture>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QTextStream>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

using glm::inverse;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::perspective;
using glm::normalize;
using glm::length;
using glm::cross;
using glm::dot;
using glm::rotate;
using glm::value_ptr;
using glm::lookAt;
//using glm::translate;

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) {
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));
    timer->start(16);
    forward = false;
    back = false;
    left = false;
    right = false;
    up = false;
    down = false;
    jumpFlag = false;
    numLayers = 1;
    heightScale = .5;
    widthScale = .625;
    depthScale = .5;

}

GLWidget::~GLWidget() {
}

void GLWidget::storeDots(){

    FILE *pacmap;
    pacmap = fopen("/home/mortinec/school/MSU-CSCI441-Programs/program3/pacgraph", "r");
    if (pacmap == NULL)
    {
        printf("\ncan't open input file\n");
        exit (1);
    }
    int i;
    int j;
    for (i = 0 ; i < 54 ; i++ ){
        for (j = 0 ; j < 54 ; j++ ){
            fscanf(pacmap,"%c", &loc[i][j]);
        }
    }
}


void GLWidget::initializeGL() {

    initializeOpenGLFunctions();
    printf("n\n");
    storeDots();

    position = vec3(-0.25, 0., 3);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glPointSize(4.0f);

    glEnable(GL_DEPTH_TEST);
    GLuint restart = 0xFFFFFFFF;
    glPrimitiveRestartIndex(restart);
    glEnable(GL_PRIMITIVE_RESTART);

    initializeCube();
    initializeBrick();
    initializePlane();

    viewMatrix = mat4(1.0f);
    modelMatrix = mat4(1.0f);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(cubeModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(planeProg);
    glUniformMatrix4fv(planeViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(planeModelMatrixLoc, 1, false, value_ptr(modelMatrix));

    glUseProgram(brickProg);
    glUniformMatrix4fv(brickViewMatrixLoc, 1, false, value_ptr(viewMatrix));
    glUniformMatrix4fv(brickModelMatrixLoc, 1, false, value_ptr(modelMatrix));
}

void GLWidget::resizeGL(int w, int h) {
    width = w;
    height = h;

    float aspect = (float)w/h;

    projMatrix = perspective(45.0f, aspect, .01f, 100.0f);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(planeProg);
    glUniformMatrix4fv(planeProjMatrixLoc, 1, false, value_ptr(projMatrix));

    glUseProgram(brickProg);
    glUniformMatrix4fv(brickProjMatrixLoc, 1, false, value_ptr(projMatrix));

}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(brickProg);
    glBindVertexArray(brickVao);
    glBindTexture(GL_TEXTURE_2D, mortarText);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,gg->width(),gg->height(),0,GL_RGBA,GL_UNSIGNED_BYTE,gg->bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    mat4 transform = glm::translate(mat4(1.0f), vec3(0,0,0));
    mat4 id = glm::translate(mat4(1.0),vec3(0,heightScale,0));
    id = id*transform;
    createWalls(id);


    mat4 s = glm::scale(mat4(1.0), vec3(.5));
    int i;
    int j;
    for (i = -27 ; i < 27 ; i++){
        for (j = -27 ; j < 27 ; j++ ){
            if(loc[i+26][j+27]=='d'){
                renderCube((glm::translate(mat4(1.0f), .25f*vec3(j,-.1, i))*s));
            }
        }
    }

    s = glm::scale(mat4(1.0), vec3(.75));

    for (i = -27 ; i < 27 ; i++){
        for (j = -27 ; j < 27 ; j++ ){
            if(loc[i+26][j+27]=='s'){
                renderCube(glm::translate(mat4(1.0f), .25f*vec3(j,-.1, i)));
            }
        }
    }


//    float angle = M_PI;
//    mat4 r =  glm::rotate(mat4(1.0),angle,vec3(1,0,0));

    for (i = -27 ; i < 27 ; i++){
        for (j = -27 ; j < 27 ; j++ ){

           renderPlane((glm::translate(mat4(1.0f), .25f*vec3(j,0, i))));

        }
    }

    glUseProgram(cubeProg);
    glBindVertexArray(cubeVao);
    glBindTexture(GL_TEXTURE_2D, textureObject);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,bricks->width(),bricks->height(),0,GL_RGBA,GL_UNSIGNED_BYTE,bricks->bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    s = glm::scale(mat4(1.0), vec3(2.5));
    for (i = -27 ; i < 27 ; i++){
        for (j = -27 ; j < 27 ; j++ ){
            if(loc[i+26][j+27]=='w'){
                renderCubeWall((glm::translate(mat4(1.0f), .25f*vec3(j,0, i))*s));
            }
        }
    }



    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,pinky->width(),pinky->height(),0,GL_RGBA,GL_UNSIGNED_BYTE,pinky->bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    for (i = -27 ; i < 27 ; i++){
            for (j = -27 ; j < 27 ; j++ ){
                if(loc[i+26][j+27]=='p'){
                    renderCubeWall((glm::translate(mat4(1.0f), .25f*vec3(j,0, i))*s));
                }
            }
        }


    glUseProgram(cubeProg);
    glBindVertexArray(cubeVao);
    glBindTexture(GL_TEXTURE_2D, textureObject);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,blinky->width(),blinky->height(),0,GL_RGBA,GL_UNSIGNED_BYTE,blinky->bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    for (i = -27 ; i < 27 ; i++){
            for (j = -27 ; j < 27 ; j++ ){
                if(loc[i+26][j+27]=='b'){
                    renderCubeWall((glm::translate(mat4(1.0f), .25f*vec3(j,0, i))*s));
                }
            }
        }


    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,clyde->width(),clyde->height(),0,GL_RGBA,GL_UNSIGNED_BYTE,clyde->bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    for (i = -27 ; i < 27 ; i++){
            for (j = -27 ; j < 27 ; j++ ){
                if(loc[i+26][j+27]=='c'){
                    renderCubeWall((glm::translate(mat4(1.0f), .25f*vec3(j,0, i))*s));
                }
            }
        }


    glUseProgram(cubeProg);
    glBindVertexArray(cubeVao);
    glBindTexture(GL_TEXTURE_2D, textureObject);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,inky->width(),inky->height(),0,GL_RGBA,GL_UNSIGNED_BYTE,inky->bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    for (i = -27 ; i < 27 ; i++){
            for (j = -27 ; j < 27 ; j++ ){
                if(loc[i+26][j+27]=='i'){
                    renderCubeWall((glm::translate(mat4(1.0f), .25f*vec3(j,0, i))*s));
                }
            }
        }

}



void GLWidget::initializePlane(){
    glGenVertexArrays(1, &planeVao);
    glBindVertexArray(planeVao);

    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    vec3 pts[] = {
        // plane
        vec3(1,-1,1),   // 0
        vec3(-1,-1,1),  // 1
        vec3(-1,-1,-1), // 2
        vec3(1,-1,-1),  // 3
    };
    for(int i = 0; i < 4; i++) {
        pts[i] *= .125;

      }
    vec3 colors[] = {
        //plane
        vec3(.3,0.3,0.3),
        vec3(.3,0.3,0.3),
        vec3(.3,0.3,0.3),
        vec3(.3,0.3,0.3)
    };

    vec2 uvs[] = {
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

    };
    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
            0,1,2,3, restart};

    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    // Load our vertex and fragment shaders into a program object
    // on the GPU
    std::cout<<"pl"<<endl;
    GLuint program = loadShaders(":/pl_vert.glsl", ":/pl_frag.glsl");
    glUseProgram(program);
    planeProg = program;

    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

     //white
     static const GLubyte tex_data[] =
     {
         0xFF
     };

    glGenTextures(1, &textureObject2);
    glBindTexture(GL_TEXTURE_2D, textureObject2);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RED,1,1,.05,GL_RED,GL_UNSIGNED_BYTE,tex_data);

    planeProjMatrixLoc = glGetUniformLocation(program, "projection");
    planeViewMatrixLoc = glGetUniformLocation(program, "view");
    planeModelMatrixLoc = glGetUniformLocation(program, "model");

}


void GLWidget::renderPlane(mat4 transform){
    glUseProgram(planeProg);
    glBindVertexArray(planeVao);
    glBindTexture(GL_TEXTURE_2D, textureObject2);
    glUniformMatrix4fv(planeModelMatrixLoc, 1, false, value_ptr(transform));
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);
}


void GLWidget::initializeCube(){
    glGenVertexArrays(1, &cubeVao);
    glBindVertexArray(cubeVao);

    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    vec3 pts[] = {
        // top
        vec3(1,1,1),    // 0
        vec3(1,1,-1),   // 1
        vec3(-1,1,-1),  // 2
        vec3(-1,1,1),   // 3

        // bottom
        vec3(1,-1,1),   // 4
        vec3(-1,-1,1),  // 5
        vec3(-1,-1,-1), // 6
        vec3(1,-1,-1),  // 7

        // front
        vec3(1,1,1),    // 8
        vec3(-1,1,1),   // 9
        vec3(-1,-1,1),  // 10
        vec3(1,-1,1),   // 11

        // back
        vec3(-1,-1,-1), // 12
        vec3(-1,1,-1),  // 13
        vec3(1,1,-1),   // 14
        vec3(1,-1,-1),  // 15

        // right
        vec3(1,-1,1),   // 16
        vec3(1,-1,-1),  // 17
        vec3(1,1,-1),   // 18
        vec3(1,1,1),     // 19

        // left
        vec3(-1,-1,1),  // 20
        vec3(-1,1,1),   // 21
        vec3(-1,1,-1),  // 22
        vec3(-1,-1,-1) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .05;
    }

    vec3 colors[] = {
        // top
        vec3(0,1,0),
        vec3(0,1,0),
        vec3(0,1,0),
        vec3(0,1,0),

        // bottom
        vec3(0,.5f,0),
        vec3(0,.5f,0),
        vec3(0,.5f,0),
        vec3(0,.5f,0),

        // front
        vec3(0,0,1),
        vec3(0,0,1),
        vec3(0,0,1),
        vec3(0,0,1),

        // back
        vec3(0,0,.5f),
        vec3(0,0,.5f),
        vec3(0,0,.5f),
        vec3(0,0,.5f),

        // right
        vec3(1,0,0),
        vec3(1,0,0),
        vec3(1,0,0),
        vec3(1,0,0),


        // left
        vec3(.5f,0,0),
        vec3(.5f,0,0),
        vec3(.5f,0,0),
        vec3(.5f,0,0)
    };

    vec2 uvs[] = {
        // top
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // bottom
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // front
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // back
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // right
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // left
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0)

    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    // Upload the position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    std::cout<<"st"<<endl;
    GLuint program = loadShaders(":/vert.glsl", ":/frag.glsl");
    glUseProgram(program);
    cubeProg = program;

    glBindBuffer(GL_ARRAY_BUFFER, cnormalBuffer);
    GLint cnormalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(cnormalIndex);
    glVertexAttribPointer(cnormalIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);
    //white

    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    GLint uvIndex = glGetAttribLocation(program, "uv");
    glEnableVertexAttribArray(uvIndex);
    glVertexAttribPointer(uvIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenTextures(1, &textureObject);
    glBindTexture(GL_TEXTURE_2D, textureObject);


    bricks = new QImage();
    if(bricks->load("/home/mortinec/school/MSU-CSCI441-Programs/program3/brick.jpg")){
        std::cout<<"loaded"<<endl;
    }

    QOpenGLTexture tex(*bricks);

    glGenTextures(1, &textureObject);
    glBindTexture(GL_TEXTURE_2D, textureObject);


    blinky = new QImage();
    if(blinky->load("/home/mortinec/school/MSU-CSCI441-Programs/program3/blinky.png")){
        std::cout<<"loaded"<<endl;
    }

    QOpenGLTexture tex1(*blinky);

    glGenTextures(1, &textureObject);
    glBindTexture(GL_TEXTURE_2D, textureObject);


    pinky = new QImage();
    if(pinky->load("/home/mortinec/school/MSU-CSCI441-Programs/program3/pinky.png")){
        std::cout<<"loaded"<<endl;
    }

    QOpenGLTexture tex2(*pinky);

    glGenTextures(1, &textureObject);
    glBindTexture(GL_TEXTURE_2D, textureObject);


    inky = new QImage();
    if(inky->load("/home/mortinec/school/MSU-CSCI441-Programs/program3/inky.png")){
        std::cout<<"loaded"<<endl;
    }

    QOpenGLTexture tex3(*inky);

    glGenTextures(1, &textureObject);
    glBindTexture(GL_TEXTURE_2D, textureObject);


    clyde = new QImage();
    if(clyde->load("/home/mortinec/school/MSU-CSCI441-Programs/program3/clyde.png")){
        std::cout<<"loaded"<<endl;
    }

    QOpenGLTexture tex4(*clyde);



    cubeProjMatrixLoc = glGetUniformLocation(program, "projection");
    cubeViewMatrixLoc = glGetUniformLocation(program, "view");
    cubeModelMatrixLoc = glGetUniformLocation(program, "model");

    cubeAlphaLoc = glGetUniformLocation(program, "alpha");
    glUniform1f(cubeAlphaLoc, 1);

    cubeLightLoc = glGetUniformLocation(program, "light");
    cubeLightAmbLoc = glGetUniformLocation(program, "amb");
    cubeCameraLoc = glGetUniformLocation(program, "cameraLoc");
    cubeShineLoc = glGetUniformLocation(program, "shiny");
    vec3 light=vec3(0,5,0);
    float amb = .1;
    float shine = 128;
    glUniform1f(cubeLightAmbLoc, GLfloat(amb) );
    glUniform3fv(cubeLightLoc,1,  value_ptr(light));
    glUniform1f(cubeShineLoc, GLfloat(shine));

}

void GLWidget::initializeBrick(){
    glGenVertexArrays(1, &brickVao);
    glBindVertexArray(brickVao);

    GLuint positionBuffer;
    glGenBuffers(1, &positionBuffer);

    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);

    glGenBuffers(1, &bnormalBuffer);

    vec3 pts[] = {
        // top
        vec3(1,1,1),    // 0
        vec3(1,1,-1),   // 1
        vec3(-1,1,-1),  // 2
        vec3(-1,1,1),   // 3

        // bottom
        vec3(1,-1,1),   // 4
        vec3(-1,-1,1),  // 5
        vec3(-1,-1,-1), // 6
        vec3(1,-1,-1),  // 7

        // front
        vec3(1,1,1),    // 8
        vec3(-1,1,1),   // 9
        vec3(-1,-1,1),  // 10
        vec3(1,-1,1),   // 11

        // back
        vec3(-1,-1,-1), // 12
        vec3(-1,1,-1),  // 13
        vec3(1,1,-1),   // 14
        vec3(1,-1,-1),  // 15

        // right
        vec3(1,-1,1),   // 16
        vec3(1,-1,-1),  // 17
        vec3(1,1,-1),   // 18
        vec3(1,1,1),     // 19

        // left
        vec3(-1,-1,1),  // 20
        vec3(-1,1,1),   // 21
        vec3(-1,1,-1),  // 22
        vec3(-1,-1,-1) // 23

    };

    for(int i = 0; i < 24; i++) {
        pts[i] *= .05;
    }

    vec3 colors[] = {
            // top
            vec3(0,1,0),
            vec3(0,1,0),
            vec3(0,1,0),
            vec3(0,1,0),

            // bottom
            vec3(0,.5f,0),
            vec3(0,.5f,0),
            vec3(0,.5f,0),
            vec3(0,.5f,0),

            // front
            vec3(0,0,1),
            vec3(0,0,1),
            vec3(0,0,1),
            vec3(0,0,1),

            // back
            vec3(0,0,.5f),
            vec3(0,0,.5f),
            vec3(0,0,.5f),
            vec3(0,0,.5f),

            // right
            vec3(1,0,0),
            vec3(1,0,0),
            vec3(1,0,0),
            vec3(1,0,0),


            // left
            vec3(.5f,0,0),
            vec3(.5f,0,0),
            vec3(.5f,0,0),
            vec3(.5f,0,0)
        };


//    for(int i = 0; i<24; i++)
//    {
//        Mortar[i] =   vec3(.25,.25,.25);
//    }

    vec2 uvs[] = {
        // top
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // bottom
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // front
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // back
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // right
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0),

        // left
        vec2(0,0),
        vec2(0,1),
        vec2(1,1),
        vec2(1,0)

    };

    GLuint restart = 0xFFFFFFFF;
    GLuint indices[] = {
        0,1,2,3, restart,
        4,5,6,7, restart,
        8,9,10,11, restart,
        12,13,14,15, restart,
        16,17,18,19, restart,
        20,21,22,23
    };

    //create(pts);

    // Upload t position data to the GPU, storing
    // it in the buffer we just allocated.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pts), pts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load our vertex and fragment shaders into a program object
    // on the GPU
    std::cout<<"br"<<endl;
    GLuint program = loadShaders(":/br_vert.glsl", ":/br_frag.glsl");
    glUseProgram(program);
    brickProg = program;

    // Bind the attribute "position" (defined in our vertex shader)
    // to the currently bound buffer object, which contains our
    // position data for a single triangle. This information
    // is stored in our vertex array object.
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLint positionIndex = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionIndex);
    glVertexAttribPointer(positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    GLint colorIndex = glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, bnormalBuffer);
    GLint normalIndex = glGetAttribLocation(program, "normal");
    glEnableVertexAttribArray(normalIndex);
    glVertexAttribPointer(normalIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    GLint uvIndex = glGetAttribLocation(program, "uv");
    glEnableVertexAttribArray(uvIndex);
    glVertexAttribPointer(uvIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);


    glGenTextures(1, &textureObject3);
    glBindTexture(GL_TEXTURE_2D, textureObject3);

    glGenTextures(1, &mortarText);
    glBindTexture(GL_TEXTURE_2D, mortarText);

    gg = new QImage();
    if(gg->load("/home/mortinec/school/MSU-CSCI441-Programs/program3/Grey_Grunge3.jpg")){
        std::cout<<"loaded"<<endl;
    }

    QOpenGLTexture tex(*gg);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    brickProjMatrixLoc = glGetUniformLocation(program, "projection");
    brickViewMatrixLoc = glGetUniformLocation(program, "view");
    brickModelMatrixLoc = glGetUniformLocation(program, "model");

}


void GLWidget::renderBrick(mat4 tran){
    glUseProgram(brickProg);
    glBindVertexArray(brickVao);

    glBindTexture(GL_TEXTURE_2D, mortarText);

    glUniformMatrix4fv(brickModelMatrixLoc, 1, false, value_ptr(tran));
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
    //renderWall();
}

void GLWidget::renderMortar(mat4 tran){
    glUseProgram(brickProg);
    glBindVertexArray(brickVao);

//    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,1,1,.05,GL_RGB,GL_UNSIGNED_BYTE,Mortar);
    glBindTexture(GL_TEXTURE_2D, mortarText);

    glUniformMatrix4fv(brickModelMatrixLoc, 1, false, value_ptr(tran));
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
    //renderWall();
}


void GLWidget::createWalls(mat4 transform){

    //printf("1");
    GLuint colorBuffer;
    glGenBuffers(1, &colorBuffer);

    mat4 scale= glm::scale(mat4(1.0f), vec3(widthScale,heightScale,depthScale ));
    backMortar(transform*scale);

    frontMortar(transform*scale);

    rightMortar(transform*scale);

    leftMortar(transform*scale);

    for(float j = -.75;j<=(.125)*heightScale;j+=.25*(heightScale)){
        mat4 translateBrick = glm::translate(mat4(1.0f),vec3(0,j,0));
        mat4 scaleBrick= glm::scale(mat4(1.0f), vec3(widthScale,heightScale,depthScale ));
        frontWall(transform*translateBrick*scaleBrick);
        rightWall(transform*translateBrick*scaleBrick);
        leftWall(transform*translateBrick*scaleBrick);
        backWall(transform*translateBrick*scaleBrick);
    }
}

void GLWidget::leftWall(mat4 transform){
    for(float i = -3; i<= 1; i+=.25)    {
        mat4 translate = glm::translate(mat4(1.0f), vec3(2.5,0.0,(i)));
        mat4 scale= glm::scale(mat4(1.0f), vec3(1.5,0.75f,1.5 ));
        brick(transform* translate*scale);
        translate = glm::translate(mat4(1.0f), vec3(2.5,.1f,(i)-.125));
        brick(transform*translate*scale);
    }
}

void GLWidget::leftMortar(mat4 transform){
    mat4  scaleMoS= glm::scale(mat4(1.0f), vec3(.75 ,15*numLayers,40));
    mat4  translate = glm::translate(mat4(1.0f), vec3(2.5,-.6*numLayers,-1));
    mortar( transform*translate*scaleMoS);
}

void GLWidget::rightWall(mat4 transform){
    for(float i = -3; i<= 1; i+=.25)    {
        mat4   translate = glm::translate(mat4(1.0f), vec3(-3.25,0.0,(i)));
        mat4   scale= glm::scale(mat4(1.0f), vec3(1.5,0.75f,1.5 ));
        brick(transform* translate*scale);
        translate = glm::translate(mat4(1.0f), vec3(-3.25,.1f,(i)-.125));
        brick(transform*translate*scale);
    }  
}

void GLWidget::rightMortar(mat4 transform){
    mat4 scaleMoS= glm::scale(mat4(1.0f), vec3(.75 ,15*numLayers,40));
    mat4 translate = glm::translate(mat4(1.0f), vec3(-3.25,-.6*numLayers,-1));
    mortar(transform* translate*scaleMoS);
}

void GLWidget::frontWall(mat4 transform){

    for(float i = -3.25; i<= 2.25; i+=.25)    {
        mat4 translate = glm::translate(mat4(1.0f), vec3((0.125f+(float)i),0,-3));
        mat4 scale= glm::scale(mat4(1.0f), vec3(1.5,0.75f,1.5 ));
        brick( transform*translate*scale);
        translate = glm::translate(mat4(1.0f), vec3(((float)i),.1,-3));
        brick(transform*translate*scale);
    }

}

void GLWidget::frontMortar(mat4 transform){
    mat4 scaleMoS= glm::scale(mat4(1.0f), vec3(58 ,15*numLayers,.75));
    mat4 translate = glm::translate(mat4(1.0f), vec3(-.375,-.6*numLayers,-3));
    mortar( transform*translate*scaleMoS);
}

void GLWidget::backWall(mat4 transform){
    for(float i = -3.25; i<= 2.25; i+=.25)    {
        mat4 translate = glm::translate(mat4(1.0f), vec3(0.0f+(float)i,0,1));
        mat4  scale= glm::scale(mat4(1.0f), vec3(1.5,0.75f,1.5 ));
        brick( transform*translate*scale);
        translate = glm::translate(mat4(1.0f), vec3(0.125f+(float)i,.1,1));
        brick(transform*translate*scale);
    }
}

void GLWidget::backMortar(mat4 transform){


    mat4  scaleMoS= glm::scale(mat4(1.0f), vec3(58 ,15*numLayers,.75));
    mat4  translate = glm::translate(mat4(1.0f), vec3(-.375,-.6*numLayers,1));
    mortar( transform*translate*scaleMoS);

}

void GLWidget::mortar(mat4 tran){
//    glUseProgram(brickProg);
//    glBindVertexArray(brickVao);
    renderMortar(tran);
}

void GLWidget::brick(mat4 tran)
{
//    glUseProgram(brickProg);
//    glBindVertexArray(brickVao);
//    glBindTexture(GL_TEXTURE_2D, mortarText);
    renderBrick(tran);
}

void GLWidget::renderCube(mat4 transform) {
    glUseProgram(cubeProg);
    glBindVertexArray(cubeVao);
    glBindTexture(GL_TEXTURE_2D, textureObject);
      static const GLubyte texwhite[]
    {
        0xFA, 0xFA, 0xFA
    };

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB,1,1,.05,GL_RGB,GL_UNSIGNED_BYTE,texwhite);

    glUniformMatrix4fv(cubeModelMatrixLoc, 1, false, value_ptr(transform));
    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
}

void GLWidget::renderCubeWall(mat4 transform){
    glUseProgram(cubeProg);
    glBindVertexArray(cubeVao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1f(cubeShineLoc,1000);
    //glUniform1f(cubeAlphaLoc, .1);

    glBindTexture(GL_TEXTURE_2D, textureObject);
    glUniformMatrix4fv(cubeModelMatrixLoc, 1, false, value_ptr(transform));

    glDrawElements(GL_TRIANGLE_FAN, 29, GL_UNSIGNED_INT, 0);
    //glUniform1f(cubeAlphaLoc, .2);

    glDisable(GL_BLEND);
}

GLuint GLWidget::loadShaders(const char* vertf, const char* fragf) {
    GLuint program = glCreateProgram();

    // read vertex shader from Qt resource file
    QFile vertFile(vertf);
    vertFile.open(QFile::ReadOnly | QFile::Text);
    QString vertString;
    QTextStream vertStream(&vertFile);
    vertString.append(vertStream.readAll());
    std::string vertSTLString = vertString.toStdString();

    const GLchar* vertSource = vertSTLString.c_str();

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSource, NULL);
    glCompileShader(vertShader);
    {
        GLint compiled;
        glGetShaderiv( vertShader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            GLsizei len;
            glGetShaderiv( vertShader, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetShaderInfoLog( vertShader, len, &len, log );
            std::cout << "Shader compilation failed: " << log << std::endl;
            delete [] log;
        }
    }
    glAttachShader(program, vertShader);

    QFile fragFile(fragf);
    fragFile.open(QFile::ReadOnly | QFile::Text);
    QString fragString;
    QTextStream fragStream(&fragFile);
    fragString.append(fragStream.readAll());
    std::string fragSTLString = fragString.toStdString();

    const GLchar* fragSource = fragSTLString.c_str();

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, NULL);
    glCompileShader(fragShader);
    {
        GLint compiled;
        glGetShaderiv( fragShader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            GLsizei len;
            glGetShaderiv( fragShader, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetShaderInfoLog( fragShader, len, &len, log );
            std::cerr << "Shader compilation failed: " << log << std::endl;
            delete [] log;
        }
    }
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    {
        GLint linked;
        glGetProgramiv( program, GL_LINK_STATUS, &linked );
        if ( !linked ) {
            GLsizei len;
            glGetProgramiv( program, GL_INFO_LOG_LENGTH, &len );

            GLchar* log = new GLchar[len+1];
            glGetProgramInfoLog( program, len, &len, log );
            std::cout << "Shader linker failed: " << log << std::endl;
            delete [] log;
        }
    }

    return program;
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
        case Qt::Key_W:
            // forward
            forward = true;
            break;
        case Qt::Key_A:
            left = true;
            // left
            break;
        case Qt::Key_S:
            back = true;
            // right
            break;
        case Qt::Key_D:
            // backward
            right = true;
            break;
        case Qt::Key_Tab:
            // toggle fly mode
            if(fly){
                fly = false;
            }else{
                fly = true;
            }
            break;
        case Qt::Key_Shift:
            // down
            down = true;
            break;
        case Qt::Key_Space:
            // up or jump
            jump = true;
            break;
    }
}

void GLWidget::keyReleaseEvent(QKeyEvent *event) {
    switch(event->key()) {
        case Qt::Key_W:
            // forward
            forward = false;
            break;
        case Qt::Key_A:
            // left
            left = false;
            break;
        case Qt::Key_S:
            // right
            back = false;
            break;
        case Qt::Key_D:
            // backward
            right = false;
            break;
        case Qt::Key_Tab:
            // toggle fly mode
            break;
        case Qt::Key_Shift:
            // down
            down = false;
            break;
        case Qt::Key_Space:
            // up or jump
            if(!fly){
                jumpFlag = true;
            }
            jump = false;
            break;
    }
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    vec2 pt(event->x(), event->y());
    lastPt = pt;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    vec2 pt(event->x(), event->y());
    vec2 d = pt-lastPt;

    yaw -= d.x/500;
    pitch -= d.y/500;

    if(pitch>M_PI/2){
        pitch = M_PI/2;
    }else if(pitch < -M_PI/2){
        pitch = -M_PI/2;
    }

    yawMatrix = glm::rotate(mat4(1.0f), yaw, vec3(0,1,0));
    pitchMatrix = glm::rotate(mat4(1.0f), pitch, vec3(1,0,0));

    orientation = yawMatrix*pitchMatrix;

    updateView();

    viewMatrix = inverse(yawMatrix*pitchMatrix);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(planeProg);
    glUniformMatrix4fv(planeViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(brickProg);
    glUniformMatrix4fv(brickViewMatrixLoc, 1, false, value_ptr(viewMatrix));

}

void GLWidget::updateView(){

    if(position.x > 8){
        position.x = -8;
    }else if(position.x < -8){
        position.x = 8;
    }

    if(position.z > 8){
        position.z = -8;
    }else if(position.z < -8){
        position.z = 8;
    }

    mat4 trans = glm::translate(glm::mat4(1.0f),position);
    viewMatrix = inverse(trans*orientation);

    glUseProgram(cubeProg);
    glUniformMatrix4fv(cubeViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(planeProg);
    glUniformMatrix4fv(planeViewMatrixLoc, 1, false, value_ptr(viewMatrix));

    glUseProgram(brickProg);
    glUniformMatrix4fv(brickViewMatrixLoc, 1, false, value_ptr(viewMatrix));

}




void GLWidget::animate(){
    float dt = .03;
    float speed = 2;
    if(length(velocity)>0){
        position += normalize(velocity)*speed*dt;
    }
    vec3 forwardVec = -vec3(yawMatrix[2]);
    vec3 rightVec = vec3(orientation[0]);
    vec3 backVec = vec3(yawMatrix[2]);
    vec3 leftVec = -vec3(orientation[0]);
    vec3 flyBackVec = vec3(orientation[2]); //fly back
    vec3 flyForwardVec = -vec3(orientation[2]); //fly forward
    vec3 flyJumpVec = vec3(orientation[1]);
    vec3 up = vec3(0,1,0);
    //float speed = 1;
    velocity = vec3(0,0,0);

    if(!fly){
        if(forward){
            velocity += forwardVec;
        }
        if(right){
            velocity+=rightVec;
        }
        if(back){
            velocity+=backVec;
        }
        if(left){
            velocity+=leftVec;
        }
        if(jump){
            velocity+=up;
        }
        if(jumpFlag){
            if(position.y>0){
                velocity-=up;
            }else if(position.y<0.1){
                jumpFlag = false;
            }
        }
        if(down){
            if(position.y>0){
                velocity-=up;
            }
            //velocity-=up;
        }
    }else{
        if(right){
            velocity+=rightVec;
        }
        if(left){
            velocity+=leftVec;
        }
        if(jump){
            velocity+=flyJumpVec;
        }
        if(forward){
            velocity+=flyForwardVec;
        }
        if(back){
            velocity+=flyBackVec;
        }
        if(down){
            velocity-=flyJumpVec;
        }
    }

    updateView();
    update();
}
