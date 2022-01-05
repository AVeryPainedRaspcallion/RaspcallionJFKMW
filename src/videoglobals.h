#pragma once

//Global Defs
typedef GLuint GL_Texture;

//SNES Gamma Ramp
uint_fast8_t gammaRamp[32] = {
    0x00, 0x01, 0x03, 0x06, 0x0a, 0x0f, 0x15, 0x1c,
    0x24, 0x2d, 0x37, 0x42, 0x4e, 0x5b, 0x69, 0x78,
    0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8, 0xc0,
    0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0xff
};

Uint32 rmask = 0x000000ff;
Uint32 gmask = 0x0000ff00;
Uint32 bmask = 0x00ff0000;
Uint32 amask = 0xff000000;

//OAM Defines
uint_fast8_t reserve_graphics_tile[6] = {
    0xE, 0xC, //Mushroom
    0x80, 0xA, //Cape
    0x68, 0xD //Flower
};
uint_fast8_t reserve_lookup[4] = {
    0,
    0x74,
    0x77,
    0x75
};
uint_fast8_t new_state_lookup[4] = {
    1, //Mushroom
    2, //Cape
    3, //Flower
    0xFF //Star
};

/*
    Trigonometrical functions (they do the triangles)
    has to do with angles and shit, radians (also do the triangles)
*/
float rad(float x) {
    return x * 3.14159f / 180.f;
}

//automatically convert to radians so I can use 0* - 359* directly
float sin_r(float x) {
    return sinf(rad(x));
}

float cos_r(float x) {
    return cosf(rad(x));
}

//Video
bool integer_scaling = true;
bool forced_scale = false;

bool fullscreen = false;
bool borderless_fullscreen = false;

bool networking = false;

//Rendering
int resolution_x = 320;
int resolution_y = 240;

SDL_Window* win;
SDL_Surface* screen_s_l1;
SDL_Surface* screen_s_l1_temp;
GL_Texture screen_t_l1GL;

SDL_Rect DestR;
SDL_Rect SrcR;

GLuint framebuffer;
GL_Texture framebuffero;
GL_Texture cached_l3_tilesGL[8];
GL_Texture cached_spr_tilesGL[16];
GL_Texture cached_bg_sprites[16];
GL_Texture DefaultGLTexture;
uint_fast8_t graphics_array[32];

SDL_Event event = { 0 };
bool show_full_screen = true;

int w; //width of the screen
int h; //height of the screen

int_fast16_t CameraX, CameraY;
int sp_offset_x = 0;
int sp_offset_y = 0;
double scale = 1.0;
uint_fast16_t int_res_x, int_res_y;
uint_fast16_t sur_res_x;

//GL Variables
uint_fast8_t opengl_r, opengl_g, opengl_b, opengl_a;
SDL_GLContext gContext;

//tex conversions
void ConvertSDLSurfaceToOpenGL(GL_Texture textureid, SDL_Surface* surface) {
    if (surface == nullptr) { cout << red << "[OpenGL] Critical: Tried to convert a texture that was nullptr." << endl; return; }

    GLenum texture_format;
    GLint  nOfColors;
    // get the number of channels in the SDL surface
    nOfColors = surface->format->BytesPerPixel;
    if (nOfColors == 4) {
        texture_format = GL_RGBA;
    }
    else if (nOfColors == 3) {
        texture_format = GL_RGB;
    }
    else {
        cout << red << "[OpenGL] Critical: Texture format is broken. nOfColors: " << nOfColors << endl;
        return;
    }
    //Gen a texture}
    glBindTexture(GL_TEXTURE_2D, textureid);
    glTexImage2D(GL_TEXTURE_2D, 0, texture_format, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //Wait for command to finish
    glFinish();
}

//used for L1
void convertL1Tex() {
    glBindTexture(GL_TEXTURE_2D, screen_t_l1GL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_s_l1->w, screen_s_l1->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, screen_s_l1->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void InitializeOpenGLViewport() {
    cout << yellow << "[SDL] Called OpenGL Initialize" << endl;

    gContext = SDL_GL_CreateContext(win);
    if (gContext == NULL) {
        cout << red << "[SDL] OpenGL context could not be created! SDL Error: " << SDL_GetError() << endl; exit(1); return;
    }
    if (SDL_GL_SetSwapInterval(1) < 0) {
        cout << red << "[SDL] Can't turn vsync on for OpenGL!: " << SDL_GetError() << endl; exit(1); return;
    }
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << red << "[GLEW] GLEW Init error!" << endl; exit(1); return;
    }

    glOrtho(0, int_res_x, int_res_y, 0, -1, 1);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenTextures(1, &screen_t_l1GL);
    glGenTextures(8, cached_l3_tilesGL);
    glGenTextures(16, cached_spr_tilesGL);
    glGenTextures(16, cached_bg_sprites);

    SDL_Surface* surf = IMG_Load("Sprites/gldefault.png");
    ConvertSDLSurfaceToOpenGL(DefaultGLTexture, surf);
    SDL_FreeSurface(surf);

    //Prepare framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glGenRenderbuffers(1, &framebuffero);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffero);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, int_res_x, int_res_y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, framebuffero);
}

void OpenGLClear() {
    //Start FBO
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, int_res_x, int_res_y);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRedraw() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, int_res_x, int_res_y, sp_offset_x, sp_offset_y, int(sp_offset_x + int_res_x * scale), int(sp_offset_y + int_res_y * scale),
        GL_COLOR_BUFFER_BIT, GL_NEAREST);

    SDL_GL_SwapWindow(win);
}

void OpenGLFillRect(SDL_Rect* Rect) {
    if (Rect == nullptr) {
        DestR = { 0, 0, int(int_res_x), int(int_res_y) }; Rect = &DestR;
    }
    int XS = Rect->x;
    int XE = Rect->x + Rect->w;
    int YS = Rect->y;
    int YE = Rect->y + Rect->h;
    glBegin(GL_QUADS);
    glColor4ub(opengl_r, opengl_g, opengl_b, opengl_a);
    glVertex2i(XS, YS); glVertex2i(XE, YS); glVertex2i(XE, YE); glVertex2i(XS, YE);
    glEnd();
}

void RenderCopyOpenGL(SDL_Rect* Rect, GL_Texture Tex, uint_fast8_t rc = 255, uint_fast8_t gc = 255, uint_fast8_t bc = 255) {
    //Bind the SDL_Texture in OpenGL
    glBindTexture(GL_TEXTURE_2D, Tex);

    //Coordinates
    int XS = Rect->x;
    int XE = Rect->x + Rect->w;
    int YS = Rect->y;
    int YE = Rect->y + Rect->h;
    glEnable(GL_TEXTURE_2D);
    glColor4ub(rc, gc, bc, 255);

    //Draw
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2i(XS, YS);
    glTexCoord2f(0, 1); glVertex2i(XS, YE);
    glTexCoord2f(1, 1); glVertex2i(XE, YE);
    glTexCoord2f(1, 0); glVertex2i(XE, YS);
    glEnd();

    //Turn off mode
    glDisable(GL_TEXTURE_2D);
}

void RenderCopyOpenGLEx(SDL_Rect* SourceRect, SDL_Rect* Rect, GL_Texture Tex, int w, int h, float rotation = 0, uint_fast8_t rc = 255, uint_fast8_t gc = 255, uint_fast8_t bc = 255) {
    //Prepare mode
    glBindTexture(GL_TEXTURE_2D, Tex);
    float XS_2 = SourceRect->x / float(w);
    float XE_2 = (SourceRect->x + SourceRect->w) / float(w);
    float YS_2 = SourceRect->y / float(h);
    float YE_2 = (SourceRect->y + SourceRect->h) / float(h);

    //Prepare texture and color
    glEnable(GL_TEXTURE_2D);
    glColor4ub(rc, gc, bc, 255);
    if (rotation != 0.f) {
        int XE = Rect->w;
        int YE = Rect->h;
        float XS = float(Rect->x + XE / 2);
        float YS = float(Rect->y + YE / 2);
        glPushMatrix();
        glTranslatef(XS, YS, 0);
        glRotatef(rotation, 0, 0, 1);
        glBegin(GL_QUADS);
        glTexCoord2f(XS_2, YS_2); glVertex2i(XE/-2, YE/-2);
        glTexCoord2f(XS_2, YE_2); glVertex2i(XE/-2, YE/2);
        glTexCoord2f(XE_2, YE_2); glVertex2i(XE/2, YE/2);
        glTexCoord2f(XE_2, YS_2); glVertex2i(XE/2, YE/-2);
        glEnd();
        glPopMatrix();
    }
    else {
        int XS = Rect->x;
        int XE = Rect->x + Rect->w;
        int YS = Rect->y;
        int YE = Rect->y + Rect->h;
        glBegin(GL_QUADS);
        glTexCoord2f(XS_2, YS_2); glVertex2i(XS, YS);
        glTexCoord2f(XS_2, YE_2); glVertex2i(XS, YE);
        glTexCoord2f(XE_2, YE_2); glVertex2i(XE, YE);
        glTexCoord2f(XE_2, YS_2); glVertex2i(XE, YS);
        glEnd();
    }

    //Turn off mode
    glDisable(GL_TEXTURE_2D);
}

void MosaicScreenSL1(uint_fast8_t mosaic_val) {
    if (mosaic_val > 0) {
        int xx = (int_res_x + 16) / mosaic_val;
        int yy = (int_res_y + 16) / mosaic_val;
        DestR = { 0, 0, xx , yy };

        SDL_LowerBlitScaled(screen_s_l1,
            NULL,
            screen_s_l1_temp,
            &DestR);
        SDL_LowerBlitScaled(screen_s_l1_temp,
            &DestR,
            screen_s_l1,
            NULL);
    }
}

//Shared Rendering
#define Ren_SetDrawColor(r, g, b, a) opengl_r = r; opengl_g = g; opengl_b = b; opengl_a = a;
#define Ren_FillRect(Rect) OpenGLFillRect(Rect);